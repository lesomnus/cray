#include <concepts>
#include <cstddef>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "cray/node.hpp"
#include "cray/props.hpp"
#include "cray/report.hpp"
#include "cray/types.hpp"

namespace cray {

namespace detail {
namespace {

template<class R, class... Args>
R invoke(R&& fallback, std::function<R(Args...)> const& f, Args&&... args) {
	if(!f) {
		return std::forward<R>(fallback);
	}

	return f(std::forward<Args>(args)...);
}

struct Callback {
	std::function<bool()> inline_note;
};

template<typename T>
void write(std::ostream& o, Interval<T> interval, std::string const& var = "x") {
	if(interval.min.has_value()) {
		o << interval.min->value
		  << (interval.min->is_inclusive ? " ≤ " : " < ");
	}

	o << var;

	if(interval.max.has_value()) {
		o << (interval.max->is_inclusive ? " ≤ " : " < ")
		  << interval.max->value;
	}
}

struct ReportContext {
	void report(Prop const& prop, Callback const& callback = Callback{}) {
		++this->level;

		auto const t = prop.type();
		switch(t) {
		case Type::Nil: this->reportScalarProp<Type::Nil>(prop, callback); break;
		case Type::Bool: this->reportScalarProp<Type::Bool>(prop, callback); break;
		case Type::Int: this->reportScalarProp<Type::Int>(prop, callback); break;
		case Type::Num: this->reportScalarProp<Type::Num>(prop, callback); break;
		case Type::Str: this->reportScalarProp<Type::Str>(prop, callback); break;
		case Type::Map: this->report(dynamic_cast<KeyedPropHolder const&>(prop), callback); break;
		case Type::List: this->report(dynamic_cast<IndexedPropHolder const&>(prop), callback); break;

		default: throw InvalidTypeError(t);
		}

		--this->level;
	}

	template<Type T>
	void reportScalarProp(Prop const& prop, Callback const& callback) {
		if constexpr(T == Type::Nil) {
			dst << "~";
			return;
		}

		auto const& p     = dynamic_cast<CodecProp<StorageOf<T>> const&>(prop);
		auto const  value = p.opt();
		if(!value.has_value()) {
			StorageOf<T> v;
			bool const   ok = p.source == nullptr ? false : p.source->get(v);
			if(ok) {
				dst << v;
			}

			if(invoke(true, callback.inline_note)) {
				dst << "  # <" << p.name() << ">";
				if(ok) {
					dst << "  ⚠️ INVALID";
				} else if(p.isNeeded()) {
					dst << "  ⚠️ REQUIRED";
				}
			}
			return;
		}

		bool const is_default_value = p.default_value.has_value() && (*p.default_value == *value);

		if constexpr(T == Type::Str) {
			if(value->find('\n') != std::string::npos) {
				auto const print_multiline = [&] {
					if(is_default_value) {
						this->enter();
					}
					this->dst << "|";

					std::istringstream s(*value);
					std::string        line;
					while(std::getline(s, line)) {
						this->enter();
						this->dst << line;
					}
				};

				if(is_default_value) {
					this->withComment(print_multiline);
				} else {
					print_multiline();
				}

				return;
			}

			if(value->find_first_of("\",(){}[]") != std::string::npos) {
				if(is_default_value) {
					this->dst << "# ";
				}
				this->dst << std::quoted(*value);

				return;
			}
		}

		if(is_default_value) {
			this->dst << "# ";
		}
		this->dst << *value;
	}

	void report(KeyedPropHolder const& prop, Callback const& callback) {
		if(prop.isMono()) {
			reportMonoMap(prop, callback);
		} else {
			reportPolyMap(prop, callback);
		}
	}

	void reportPolyMap(KeyedPropHolder const& prop, Callback const& callback) {
		std::shared_ptr<Source> source;
		if(prop.hasDefault() && !prop.source->is(Type::Map)) {
			source = Source::make(nullptr);
			prop.encodeDefaultValueInto(*source);
			assert(source->is(Type::Map));
		} else {
			source = prop.source;
		}

		prop.forEachProps(*source, [&](std::string const& key, std::shared_ptr<Prop> const& next_prop) {
			bool const is_annotated = this->annotate(*next_prop);
			this->enter();

			this->dst << key << ": ";
			this->report(*next_prop);

			if(is_annotated) {
				this->enter();
			}

			return true;
		});
	}

	void reportMonoMap(KeyedPropHolder const& prop, Callback const& callback) {
		auto const next_prop = prop.at(Reference());
		if(next_prop == nullptr) {
			return;
		}

		this->annotate(*next_prop);

		for(auto const& key: prop.required_keys) {
			if(prop.source->has(key)) {
				continue;
			}

			this->enter();
			this->dst << key << ":   # <" << next_prop->name() << ">  ⚠️ REQUIRED";
		}

		std::size_t cnt = 0;
		prop.forEachProps([&](std::string const& key, std::shared_ptr<Prop> const& next_prop) {
			++cnt;

			this->enter();
			this->dst << key << ": ";
			this->report(*next_prop);
		});

		if(cnt == 0) {
			this->withComment([&] {
				next_prop->source = Source::null();

				this->enter();
				this->dst << "key: ";
				this->report(*next_prop);
				this->enter();
				this->dst << "...";
			});
		}
	}

	void report(IndexedPropHolder const& prop, Callback const& callback) {
		if(prop.isMono()) {
			reportMonoList(prop, callback);
		} else {
			throw std::runtime_error("not implemented");
		}
	}

	void reportMonoList(IndexedPropHolder const& prop, Callback const& callback) {
		auto const next_prop = prop.at(Reference());
		if(next_prop == nullptr) {
			return;
		}

		bool is_default_value = false;

		std::shared_ptr<Source> source;
		if(prop.hasDefault() && !prop.source->is(Type::List)) {
			is_default_value = true;

			source = Source::make(nullptr);
			prop.encodeDefaultValueInto(*source);
			assert(source->is(Type::List));
		} else {
			source = prop.source;
		}

		if(source->isEmpty()) {
			this->withComment([&] {
				next_prop->source = Source::null();

				this->enter();
				this->dst << "- ";
				this->report(*next_prop);
				this->enter();
				this->dst << "- ...";

				auto const interval = prop.interval();
				if(interval.isSingleton()) {
					auto const num = interval.minimum();
					if(num > 3) {
						this->enter();
						this->dst << "- (" << (num - 2) << " items more)";
					} else if(num == 3) {
						this->enter();
						this->dst << "- ...";
					}
				}
			});

			return;
		}

		auto const type = next_prop->type();

		bool is_multiline = !isScalarType(type);
		if(type == Type::Str) {
			std::string v;

			std::size_t const size = source->size();
			for(std::size_t i = 0; i < size; ++i) {
				source->next(i)->get(v);
				if(v.find('\n') != std::string::npos) {
					is_multiline = true;
					break;
				}
			}
		}

		if(is_multiline) {
			annotate(*next_prop);

			std::size_t const cnt_elems = source->size();
			for(std::size_t i = 0; i < cnt_elems; ++i) {
				this->enter();
				this->dst << "- ";

				next_prop->source = source->next(i);
				next_prop->ref    = i;
				this->report(*next_prop);
			}

			return;
		}

		bool const ok = is_default_value || prop.ok();

		bool const is_annotated = annotate(*next_prop);
		if(is_annotated || this->level <= 0) {
			this->enter();
		}
		if(is_default_value) {
			this->dst << "# ";
		}
		this->dst << "[";

		bool is_first = true;

		std::size_t const cnt_elems = source->size();
		for(std::size_t i = 0; i < cnt_elems; ++i) {
			if(!std::exchange(is_first, false)) {
				this->dst << ", ";
			}

			next_prop->source = source->next(i);
			next_prop->ref    = i;

			this->report(*next_prop, Callback{.inline_note = []() -> bool {
				return false;
			}});
		}

		this->dst << "]";

		if(!ok) {
			this->dst << "  ⚠️ INVALID";
		}
	}

	void annotate(Annotation const& annotation, std::function<void()> const& on_annotate) {
		if(!annotation.title.empty()) {
			on_annotate();
			this->enter();
			this->dst << "# " << annotation.title;
		}

		if(annotation.is_deprecated) {
			on_annotate();
			this->enter();
			this->dst << "# ⚠️ DEPRECATED";
		}

		if(!annotation.description.empty()) {
			on_annotate();
			this->enter();

			if(annotation.description.find('\n') == std::string::npos) {
				this->dst << "# | " << annotation.description;
			} else {
				this->dst << "# | ";
				bool is_first = true;

				std::istringstream s(annotation.description);
				std::string        line;
				while(std::getline(s, line)) {
					if(!std::exchange(is_first, false)) {
						this->enter();
						this->dst << "#   ";
					}

					this->dst << line;
				}
			}
		}
	}

	void annotate(Prop const& prop, std::function<void()> const& on_annotate) {
		annotate(prop.annotation, on_annotate);

		switch(prop.type()) {
		case Type::Nil: this->annotate(dynamic_cast<NilProp const&>(prop), on_annotate); return;
		case Type::Bool: this->annotate(dynamic_cast<BoolProp const&>(prop), on_annotate); return;
		case Type::Int: this->annotateNumeric(dynamic_cast<NumericProp<Type::Int> const&>(prop), on_annotate); return;
		case Type::Num: this->annotateNumeric(dynamic_cast<NumericProp<Type::Num> const&>(prop), on_annotate); return;
		case Type::Str: this->annotate(dynamic_cast<StrProp const&>(prop), on_annotate); return;
		case Type::Map: return;
		case Type::List: this->annotate(dynamic_cast<IndexedPropHolder const&>(prop), on_annotate); return;

		default: throw std::runtime_error("invalid type");
		}
	}

	bool annotate(Prop const& prop) {
		bool is_annotate = false;
		this->annotate(prop, [&] { is_annotate = true; });
		return is_annotate;
	}

	template<Type T>
	requires((T == Type::Int) || (T == Type::Num))
	void annotateNumeric(NumericProp<T> const& prop, std::function<void()> const& on_annotate) {
		bool const has_interval = !prop.interval.isAll();
		bool const has_divisor  = !prop.multiple_of.empty();
		if(has_interval || has_divisor) {
			on_annotate();
			this->enter();

			if constexpr(T == Type::Int) {
				this->dst << "# • { x ∈ Z | ";
			} else {
				this->dst << "# • { x ∈ Q | ";
			}
			if(has_interval && has_divisor) {
				this->dst << "(";
				write(this->dst, prop.interval);
				this->dst << ") ∧ (x / " << prop.multiple_of.divisor << " ∈ Z)";
			} else if(has_interval) {
				write(this->dst, prop.interval);
			} else if(has_divisor) {
				this->dst << "x / " << prop.multiple_of.divisor << " ∈ Z";
			}
			this->dst << " }";

			if(prop.with_clamp) {
				this->enter();
				this->dst << "# • value is clamped if it is not in interval";
			}
		}
	}

	void annotate(NilProp const& prop, std::function<void()> const& on_annotate) {
	}

	void annotate(BoolProp const& prop, std::function<void()> const& on_annotate) {
	}

	void annotate(StrProp const& prop, std::function<void()> const& on_annotate) {
		if(!prop.allowed_values.empty()) {
			on_annotate();
			this->enter();

			this->dst << "# • ";

			bool is_first = true;
			for(auto const& value: prop.allowed_values) {
				if(!std::exchange(is_first, false)) {
					this->dst << " | ";
				}

				this->dst << value;
			}
		}

		if(!prop.length.isAll()) {
			on_annotate();
			this->enter();

			this->dst << "# • { |X| ∈ W | ";
			write(this->dst, prop.length, "|X|");
			this->dst << " }";
		}
	}

	void annotate(IndexedPropHolder const& prop, std::function<void()> const& on_annotate) {
		auto const interval = prop.interval();
		if(!interval.isAll()) {
			on_annotate();
			this->enter();

			if(interval.isSingleton()) {
				this->dst << "# • |X| is " << interval.minimum();
			} else {
				this->dst << "# • { |X| ∈ W | ";
				write(this->dst, interval, "|X|");
				this->dst << " }";
			}
		}
	}

	void tab() const {
		if(this->comment_blocks.empty()) {
			if(this->level > 0) {
				this->dst << std::string(this->tab_size * this->level, ' ');
			}
		} else {
			auto const  size = this->comment_blocks.size();
			std::string t((this->tab_size * this->level) + (2 * size), ' ');

			auto cnt = 0;
			for(std::size_t cnt = 0; cnt < size; ++cnt) {
				auto const l = this->comment_blocks.at(cnt);

				t.at((this->tab_size * l) + (2 * cnt)) = '#';
			}

			this->dst << t;
		}
	}

	void withComment(std::function<void()> const& functor) {
		this->comment_blocks.emplace_back(this->level);
		functor();
		this->comment_blocks.pop_back();
	}

	void enter() {
		this->dst << std::endl;
		this->tab();
	}

	std::ostream& dst;

	std::size_t tab_size = 2;  // must be positive.
	int         level    = -1;

	std::vector<int> comment_blocks;
};

class YamlReporter: public Reporter {
   protected:
	void report_(std::ostream& dst, detail::Prop const& prop) const override {
		auto ctx = detail::ReportContext{.dst = dst};

		dst << std::boolalpha;
		dst << "---";

		ctx.annotate(prop);
		if(isScalarType(prop.type())) {
			ctx.enter();
		}

		ctx.report(prop);
	}
};

}  // namespace
}  // namespace detail

namespace report {

void asYaml(std::ostream& dst, Node node) {
	detail::YamlReporter{}.report(dst, node);
}

}  // namespace report
}  // namespace cray

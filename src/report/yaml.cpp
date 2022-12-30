#include <cstddef>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>

#include "cray/node.hpp"
#include "cray/props.hpp"
#include "cray/report.hpp"
#include "cray/types.hpp"

namespace cray {

namespace detail {
namespace {

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
	void report(Prop const& prop) {
		const auto t = prop.type();
		switch(t) {
		case Type::Nil: this->reportScalarProp<Type::Nil>(prop); return;
		case Type::Bool: this->reportScalarProp<Type::Bool>(prop); return;
		case Type::Int: this->reportScalarProp<Type::Int>(prop); return;
		case Type::Num: this->reportScalarProp<Type::Num>(prop); return;
		case Type::Str: this->reportScalarProp<Type::Str>(prop); return;
		case Type::Map: this->report(dynamic_cast<KeyedPropHolder const&>(prop)); return;
		case Type::List: this->reportList(prop); return;

		default: throw InvalidTypeError(t);
		}
	}

	template<Type T>
	void reportScalarProp(Prop const& prop) {
		if constexpr(T == Type::Nil) {
			dst << "~";
			return;
		}

		auto const& p     = dynamic_cast<CodecProp<StorageOf<T>> const&>(prop);
		auto const  value = p.opt();
		if(!value.has_value()) {
			dst << "  # <" << p.name() << ">";
			if(p.isNeeded()) {
				dst << "  ⚠️ REQUIRED";
			}
			return;
		}

		if(p.default_value.has_value() && (*p.default_value == *value)) {
			dst << "# ";
		}

		if constexpr(T == Type::Bool) {
			dst << (*value ? "true" : "false");
		} else {
			dst << *value;
		}
	}

	void report(KeyedPropHolder const& prop) {
		auto const s = this->scope();
		if(this->level > 0) {
			this->dst << std::endl;
		}

		if(prop.isConcrete()) {
			reportPolyMap(prop);
		} else {
			reportMonoMap(prop);
		}
	}

	void reportPolyMap(KeyedPropHolder const& prop) {
		auto const& p = dynamic_cast<KeyedPropHolder const&>(prop);

		std::shared_ptr<Source> source;
		if(prop.hasDefault() && !prop.source->is(Type::Map)) {
			source = Source::make(nullptr);
			prop.encodeDefaultValueInto(*source);
			assert(source->is(Type::Map));
		} else {
			source = prop.source;
		}

		bool is_first = true;
		p.forEachProps(*source, [&](std::string const& key, std::shared_ptr<Prop> const& next_prop) {
			if(!std::exchange(is_first, false)) {
				this->dst << std::endl;
			}

			bool is_annotated = false;
			annotate(*next_prop, [&] { is_annotated = true; });

			this->tab();
			this->dst << key << ": ";
			this->report(*next_prop);

			if(is_annotated) {
				this->dst << std::endl;
			}

			return true;
		});
	}

	void reportMonoMap(KeyedPropHolder const& prop) {
		auto const next_prop = prop.at(Reference());
		if(next_prop == nullptr) {
			return;
		}

		this->annotate(*next_prop, [] {});

		for(auto const& key: prop.required_keys) {
			if(prop.source->has(key)) {
				continue;
			}

			this->tab();
			this->dst << key << ":   # <" << next_prop->name() << ">  ⚠️ REQUIRED";
			this->dst << std::endl;
		}

		bool is_first = true;
		prop.forEachProps([&](std::string const& key, std::shared_ptr<Prop> const& next_prop) {
			if(!std::exchange(is_first, false)) {
				this->dst << std::endl;
			}

			this->tab();
			this->dst << key << ": ";
			this->report(*next_prop);
		});

		if(is_first) {
			this->tab();
			this->dst << "# key: <" << next_prop->name() << ">";
		}
	}

	void reportList(Prop const& prop) {
		auto const s = this->scope();

		reportMonoList(prop);
	}

	void reportMonoList(Prop const& prop) {
		auto const next_prop = prop.at(Reference());
		if(next_prop == nullptr) {
			return;
		}

		std::shared_ptr<Source> source;
		if(prop.hasDefault() && !prop.source->is(Type::List)) {
			this->dst << "# ";

			source = Source::make(nullptr);
			prop.encodeDefaultValueInto(*source);
			assert(source->is(Type::List));
		} else {
			source = prop.source;
		}

		if(source->isEmpty()) {
			this->tab();
			this->dst << "# - <" << next_prop->name() << ">" << std::endl;
			this->tab();
			this->dst << "# - ...";
			return;
		}

		if(isScalarType(next_prop->type())) {
			bool is_annotated = false;
			annotate(*next_prop, [this, &is_annotated]() mutable {
				if(!is_annotated) {
					is_annotated = true;
					this->dst << std::endl;
				}
			});

			if(is_annotated) {
				this->tab();
			}

			this->dst << "[";

			std::size_t const cnt_elems = source->size();
			for(std::size_t i = 0; i < cnt_elems; ++i) {
				next_prop->source = source->next(i);
				next_prop->ref    = i;
				this->report(*next_prop);

				if(i == (cnt_elems - 1)) {
					break;
				}

				this->dst << ", ";
			}

			this->dst << "]";
		} else {
			if(this->level > 0) {
				this->dst << std::endl;
			}

			annotate(*next_prop, [] {});

			std::size_t const cnt_elems = source->size();
			for(std::size_t i = 0; i < cnt_elems; ++i) {
				this->tab();
				this->dst << "- ";

				next_prop->source = source->next(i);
				next_prop->ref    = i;
				this->report(*next_prop);

				if(i == (cnt_elems - 1)) {
					break;
				}

				this->dst << std::endl;
			}
		}
	}

	void annotate(Annotation const& annotation, std::function<void()> const& on_annotate) {
		if(!annotation.title.empty()) {
			on_annotate();
			this->tab();
			this->dst << "# " << annotation.title << std::endl;
		}

		if(annotation.is_deprecated) {
			on_annotate();
			this->tab();
			this->dst << "# ⚠️ DEPRECATED" << std::endl;
		}

		if(!annotation.description.empty()) {
			on_annotate();
			// TODO: handle multiline.
			this->tab();
			this->dst << "# | " << annotation.description << std::endl;
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
		case Type::List: this->annotateList(prop, on_annotate); return;

		default: throw std::runtime_error("invalid type");
		}
	}

	template<Type T>
	    requires((T == Type::Int) || (T == Type::Num))
	void annotateNumeric(NumericProp<T> const& prop, std::function<void()> const& on_annotate) {
		bool const has_interval = !prop.interval.isAll();
		bool const has_divisor  = !prop.multiple_of.empty();
		if(has_interval || has_divisor) {
			on_annotate();
			this->tab();
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
			this->dst << " }" << std::endl;

			if(prop.with_clamp) {
				this->tab();
				this->dst << "# • value is clamped if it is not in interval" << std::endl;
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
			this->tab();
			this->dst << "# • ";

			std::size_t cnt_remain = prop.allowed_values.size();
			for(auto const& value: prop.allowed_values) {
				this->dst << value;
				if(--cnt_remain > 0) {
					this->dst << " | ";
				}
			}
			this->dst << std::endl;
		}

		if(!prop.length.isAll()) {
			on_annotate();
			this->tab();
			this->dst << "# • { |x| ∈ W | ";
			write(this->dst, prop.length, "|x|");
			this->dst << " }" << std::endl;
		}
	}

	void annotateList(Prop const& prop, std::function<void()> const& on_annotate) {
		if(auto const* p = dynamic_cast<MonoListAccessor const*>(&prop); p != nullptr) {
			annotateMonoList(prop, on_annotate);
		} else {
			throw std::runtime_error("not implemented");
		}
	}

	void annotateMonoList(Prop const& prop, std::function<void()> const& on_annotate) {
		auto const& accessor = dynamic_cast<MonoListAccessor const&>(prop);
		if(auto const& size = accessor.getSize(); !size.isAll()) {
			on_annotate();
			this->tab();
			this->dst << "# • { |x| ∈ W | ";
			write(this->dst, size, "|x|");
			this->dst << " }" << std::endl;
		}
	}

	void tab() const {
		if(this->level <= 0) {
			return;
		}

		this->dst << std::string(this->tab_size * this->level, ' ');
	}

	std::shared_ptr<void> scope() {
		++this->level;
		return std::shared_ptr<void>(nullptr, [this](...) {
			--this->level;
		});
	}

	std::ostream& dst;

	std::size_t tab_size = 2;
	int         level    = -1;
};

class YamlReporter: public Reporter {
   protected:
	void report_(std::ostream& dst, detail::Prop const& prop) const override {
		auto ctx = detail::ReportContext{.dst = dst};
		ctx.annotate(prop, [] {});
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

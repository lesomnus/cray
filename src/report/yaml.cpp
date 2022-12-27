#include <cstddef>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>

#include <ranges>

#include "cray/detail/props.hpp"
#include "cray/node.hpp"
#include "cray/report.hpp"
#include "cray/types.hpp"

namespace cray {

namespace detail {
namespace {

template<std::integral T>
struct ScopedIncrement {
	ScopedIncrement(T& value)
	    : value(++value) { }

	~ScopedIncrement() {
		--this->value;
	}

	T& value;
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
	void report(Prop const& prop) {
		switch(prop.type()) {
		case Type::Nil: this->reportScalarProp<Type::Nil>(prop); return;
		case Type::Bool: this->reportScalarProp<Type::Bool>(prop); return;
		case Type::Int: this->reportScalarProp<Type::Int>(prop); return;
		case Type::Num: this->reportScalarProp<Type::Num>(prop); return;
		case Type::Str: this->reportScalarProp<Type::Str>(prop); return;
		case Type::Map: this->reportMap(prop); return;
		case Type::List: this->reportList(prop); return;

		default: throw std::runtime_error("invalid type");
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
			if(p.isNeeded()) {
				dst << "  # ⚠️ REQUIRED";
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

	void reportMap(Prop const& prop) {
		auto i = ScopedIncrement(this->level);
		if(this->level > 0) {
			this->dst << std::endl;
		}

		if(auto const* p = dynamic_cast<PolyMpaProp const*>(&prop); p != nullptr) {
			report(*p);
		} else if(dynamic_cast<MonoMapPropAccessor const*>(&prop) != nullptr) {
			reportMonoMap(prop);
		} else {
			throw std::runtime_error("not implemented");
		}
	}

	void report(PolyMpaProp const& prop) {
		auto i = prop.next_props.size();
		for(auto const& [key, next_prop]: prop.next_props) {
			annotate(*next_prop, [] {});
			this->tab();
			this->dst << key << ": ";
			this->report(*next_prop);

			if(--i > 0) {
				this->dst << std::endl;
			}
		}
	}

	void reportMonoMap(Prop const& prop) {
		auto const next_prop = prop.at(Reference());
		if(next_prop == nullptr) {
			return;
		}

		annotate(*next_prop, [] {});

		// Check required fields
		{
			auto const& accessor      = dynamic_cast<MonoMapPropAccessor const&>(prop);
			auto const& required_keys = accessor.getRequiredKeys();

			auto cnt_remain_required_keys = required_keys.size();
			if(prop.source->size() > 0) {
				cnt_remain_required_keys += 1;
			}

			for(auto const& key: required_keys | std::views::drop_while(HeldBy(*prop.source))) {
				this->tab();
				this->dst << key << ":   # ⚠️ REQUIRED";

				if(--cnt_remain_required_keys > 0) {
					this->dst << std::endl;
				}
			}
		}

		std::size_t cnt_remain = prop.source->size();
		prop.source->keys([&](std::string const& key) {
			this->tab();
			this->dst << key << ": ";

			next_prop->source = prop.source->next(key);
			next_prop->ref    = key;
			this->report(*next_prop);

			if(--cnt_remain > 0) {
				this->dst << std::endl;
			}

			return true;
		});
	}

	void reportList(Prop const& prop) {
		auto i = ScopedIncrement(this->level);

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

		if(isScalarType(next_prop->type())) {
			annotate(*next_prop, [this, called = false]() mutable {
				if(!called) {
					called = true;
					this->dst << std::endl;
				}
			});

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

	void tabOnce() const {
		this->dst << std::string(this->tab_size, ' ');
	}

	std::ostream& dst;

	std::size_t tab_size = 2;
	int         level    = -1;
};

}  // namespace
}  // namespace detail

namespace report {

namespace {

class YamlReporter: public Reporter {
   protected:
	void report_(std::ostream& dst, detail::Prop const& prop) const override {
		auto ctx = detail::ReportContext{.dst = dst};
		ctx.annotate(prop, [] {});
		ctx.report(prop);
	}
};

}  // namespace

void asYaml(std::ostream& dst, Node node) {
	YamlReporter{}.report(dst, node);
}

}  // namespace report
}  // namespace cray

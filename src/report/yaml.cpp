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
void write(std::ostream& o, Interval<T> interval) {
	if(interval.min.has_value()) {
		o << interval.min->value
		  << (interval.min->is_inclusive ? " ≤ " : " < ");
	}

	o << "x";

	if(interval.max.has_value()) {
		o << (interval.max->is_inclusive ? " ≤ " : " < ")
		  << interval.max->value;
	}
}

struct ReportContext {
	void report(Prop const& prop) {
		switch(prop.type()) {
		case Type::Nil: return;
		case Type::Bool: return;
		case Type::Int: this->report(dynamic_cast<IntProp const&>(prop)); return;
		case Type::Num: return;
		case Type::Str: return;
		case Type::Map: this->reportMap(prop); return;
		case Type::List: return;

		default: throw std::runtime_error("invalid type");
		}
	}

	void report(IntProp const& prop) {
		auto const value = prop.opt();
		if(!value.has_value()) {
			if(prop.isNeeded()) {
				dst << "  # ⚠️ REQUIRED";
			}
			return;
		}

		if(prop.default_value.has_value() && (*prop.default_value == *value)) {
			dst << "# ";
		}

		dst << *value;
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
			annotate(*next_prop);
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

		annotate(*next_prop);

		// Check required fields
		{
			auto const& accessor      = dynamic_cast<MonoMapPropAccessor const&>(prop);
			auto const& required_keys = accessor.requiredKeys();

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

		auto cnt_remain = prop.source->size();
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

	void annotate(Annotation const& annotation) {
		if(!annotation.title.empty()) {
			this->tab();
			this->dst << "# " << annotation.title << std::endl;
		}

		if(annotation.is_deprecated) {
			this->tab();
			this->dst << "# ⚠️ DEPRECATED" << std::endl;
		}

		if(!annotation.description.empty()) {
			// TODO: handle multiline.
			this->tab();
			this->dst << "# | " << annotation.description << std::endl;
		}
	}

	void annotate(Prop const& prop) {
		annotate(prop.annotation);
		switch(prop.type()) {
		case Type::Int: annotate(dynamic_cast<IntProp const&>(prop)); return;

		case Type::Map: return;

		default: throw std::runtime_error("not implemented");
		}
	}

	void annotate(IntProp const& prop) {
		bool const has_interval = !prop.interval.isAll();
		bool const has_divisor  = !prop.multiple_of.empty();
		if(has_interval || has_divisor) {
			this->tab();
			this->dst << "# • { x ∈ Z | ";
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
		ctx.annotate(prop);
		ctx.report(prop);
	}
};

}  // namespace

void asYaml(std::ostream& dst, Node node) {
	YamlReporter{}.report(dst, node);
}

}  // namespace report
}  // namespace cray

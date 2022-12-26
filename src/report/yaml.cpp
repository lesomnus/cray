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
		annotate(prop);

		switch(prop.type()) {
		case Type::Nil: return;
		case Type::Bool: return;
		case Type::Int: this->reportInt(prop); return;
		case Type::Num: return;
		case Type::Str: return;
		case Type::Map: this->reportMap(prop); return;
		case Type::List: return;

		default: throw std::runtime_error("invalid type");
		}
	}

	void reportInt(Prop const& prop) {
		auto* p = dynamic_cast<IntProp const*>(&prop);
		if(p == nullptr) {
			return;
		}

		auto const value = p->opt();
		if(!value.has_value()) {
			if(p->isNeeded()) {
				dst << "  # ⚠️ REQUIRED";
			} else {
				dst << "  # ⚠️ INVALID VALUE";
			}

			return;
		}

		if(p->default_value.has_value() && (*p->default_value == *value)) {
			dst << "# ";
		}

		dst << *value;
	}

	void reportMap(Prop const& prop) {
		if(dynamic_cast<MonoMapPropAccessor const*>(&prop) != nullptr) {
			reportMonoMap(prop);
		} else {
			throw std::runtime_error("not implemented");
		}
	}

	void reportMonoMap(Prop const& prop) {
		auto const next = prop.at(Reference());
		if(next == nullptr) {
			return;
		}

		annotate(*next);

		std::function<void()> const report_next = ([&] {
			switch(next->type()) {
				// case Type::Nil: return;
				// case Type::Bool: return;

			case Type::Int: {
				return [&] {
					this->reportInt(*next);
					this->dst << std::endl;
				};
			}

				// case Type::Num: return;
				// case Type::Str: return;
				// case Type::Map: this->reportMap(prop); return;
				// case Type::List: return;

			default: throw std::runtime_error("not implemented");
			}
		})();

		// Check required fields
		{
			auto const& accessor = dynamic_cast<MonoMapPropAccessor const&>(prop);
			for(auto const& key: accessor.requiredKeys() | std::views::drop_while(HeldBy(*prop.source))) {
				tab();
				this->dst << key << ":   # ⚠️ REQUIRED" << std::endl;
			}
		}

		prop.source->keys([&](std::string const& key) {
			tab();
			this->dst << key << ": ";
			next->source = prop.source->next(key);
			next->ref    = key;
			report_next();

			return true;
		});
	}

	void annotate(Annotation const& annotation) {
		if(!annotation.title.empty()) {
			tab();
			this->dst << "# " << annotation.title << std::endl;
		}

		if(annotation.is_deprecated) {
			tab();
			this->dst << "# ⚠️ DEPRECATED" << std::endl;
		}

		if(!annotation.description.empty()) {
			// TODO: handle multiline.
			tab();
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
			tab();
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
				tab();
				this->dst << "# • value is clamped if it is not in interval" << std::endl;
			}
		}
	}

	void tab() const {
		if(this->level == 0) {
			return;
		}

		this->dst << std::string(' ', this->tab_size * this->level);
	}

	void tabOnce() const {
		this->dst << std::string(' ', this->tab_size);
	}

	std::ostream& dst;

	std::size_t tab_size = 4;
	std::size_t level    = 0;
};

}  // namespace
}  // namespace detail

namespace report {

namespace {

class YamlReporter: public Reporter {
   protected:
	void report_(std::ostream& dst, detail::Prop const& prop) const override {
		detail::ReportContext{.dst = dst}.report(prop);
	}
};

}  // namespace

void asYaml(std::ostream& dst, Node node) {
	YamlReporter{}.report(dst, node);
}

}  // namespace report
}  // namespace cray

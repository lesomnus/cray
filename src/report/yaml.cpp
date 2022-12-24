#include <cstddef>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "cray/detail/props.hpp"
#include "cray/node.hpp"
#include "cray/report.hpp"
#include "cray/types.hpp"

namespace cray {

namespace detail {
namespace {

struct ReportContext {
	void report(Prop const& prop) {
		annotate(prop.annotation);

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
			return;
		}

		dst << *value;
	}

	void reportMap(Prop const& prop) {
		reportMonoMap(prop);
	}

	void reportMonoMap(Prop const& prop) {
		auto const next = prop.at(Reference());
		if(next == nullptr) {
			return;
		}

		std::function<void()> const report_next = ([&] {
			switch(next->type()) {
				// case Type::Nil: return;
				// case Type::Bool: return;

			case Type::Int:
				return [&] {
					this->reportInt(*next);
					this->dst << std::endl;
				};

				// case Type::Num: return;
				// case Type::Str: return;
				// case Type::Map: this->reportMap(prop); return;
				// case Type::List: return;

			default: throw std::runtime_error("not implemented");
			}
		})();

		auto const keys = prop.source->keys();
		for(auto const& key: keys) {
			// TODO: Quote if needed.
			this->dst << key << ": ";
			next->source = prop.source->next(key);
			report_next();
		}
	}

	void tab() const {
		if(this->level == 0) {
			return;
		}

		this->dst << std::string(' ', this->tab_size * this->level);
	}

	void annotate(Annotation const& annotation) {
		if(!annotation.title.empty()) {
			tab();
			this->dst << "# " << annotation.title << std::endl;
		}

		if(!annotation.description.empty()) {
			// TODO: handle multiline.
			tab();
			this->dst << "# " << annotation.description << std::endl;
		}
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

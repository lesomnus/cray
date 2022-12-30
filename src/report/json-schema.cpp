#include <array>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#include "cray/props.hpp"
#include "cray/report.hpp"
#include "cray/types.hpp"

namespace cray {

namespace detail {
namespace {

struct ReportContext {
	void report(Prop const& prop) {
		auto const t = prop.type();
		this->field("type") << std::quoted(this->toString(t));

		if(!prop.annotation.title.empty()) {
			this->field("title") << std::quoted(prop.annotation.title);
		}

		if(!prop.annotation.description.empty()) {
			this->field("description") << std::quoted(prop.annotation.description);
		}

		if(prop.annotation.is_deprecated) {
			this->field("deprecated") << prop.annotation.is_deprecated;
		}

		switch(t) {
		case Type::Nil: this->report(dynamic_cast<NilProp const&>(prop)); break;
		case Type::Bool: this->report(dynamic_cast<BoolProp const&>(prop)); break;
		case Type::Int: this->report(dynamic_cast<NumericProp<Type::Int> const&>(prop)); break;
		case Type::Num: this->report(dynamic_cast<NumericProp<Type::Num> const&>(prop)); break;
		case Type::Str: this->report(dynamic_cast<StrProp const&>(prop)); break;
		case Type::Map: this->report(dynamic_cast<KeyedPropHolder const&>(prop)); break;
		case Type::List: this->report(dynamic_cast<IndexedPropHolder const&>(prop)); break;

		default: throw InvalidTypeError(t);
		}
	}

	void report(NilProp const& prop) {
	}

	void report(BoolProp const& prop) {
	}

	void report(NumericProp<Type::Int> const& prop) {
		if(!prop.multiple_of.empty()) {
			this->field("multipleOf") << prop.multiple_of.divisor;
		}

		if(!prop.interval.isAll()) {
			this->reportNumericInterval(prop.interval);
		}
	}

	void report(NumericProp<Type::Num> const& prop) {
		if(!prop.multiple_of.empty()) {
			this->field("multipleOf") << prop.multiple_of.divisor;
		}

		if(!prop.interval.isAll()) {
			this->reportNumericInterval(prop.interval);
		}
	}

	void report(StrProp const& prop) {
		if(!prop.allowed_values.empty()) {
			this->fieldA("enum", [&] {
				for(auto const& value: prop.allowed_values) {
					this->value() << std::quoted(value);
				}
			});
		}

		if(!prop.length.isAll()) {
			this->reportLengthInterval(prop.length);
		}
	}

	void report(KeyedPropHolder const& prop) {
		if(!prop.required_keys.empty()) {
			this->fieldA("required", [&] {
				for(auto const& key: prop.required_keys) {
					this->value() << std::quoted(key);
				}
			});
		}

		if(prop.isConcrete()) {
			this->fieldO("properties", [&] {
				prop.forEachProps([this](std::string const& key, std::shared_ptr<Prop> const& next_prop) {
					this->fieldO(key, [&] {
						this->report(*next_prop);
					});
				});
			});
		} else {
			this->fieldO("additionalProperties", [&] {
				auto const next_prop = prop.at(Reference());
				this->report(*next_prop);
			});
		}
	}

	void report(IndexedPropHolder const& prop) {
		this->fieldO("items", [&] {
			auto const next_prop = prop.at(Reference());
			this->report(*next_prop);
		});
	}

	template<typename T>
	void reportNumericInterval(Interval<T> const& interval) {
		if(interval.min.has_value()) {
			this->field("minimum") << interval.min->value;
			if(!interval.min->is_inclusive) {
				this->field("exclusiveMinimum") << true;
			}
		}

		if(interval.max.has_value()) {
			this->field("maximum") << interval.max->value;
			if(!interval.max->is_inclusive) {
				this->field("exclusiveMaximum") << true;
			}
		}
	}

	template<std::integral T>
	void reportLengthInterval(Interval<T> const& interval) {
		if(interval.min.has_value()) {
			auto value = interval.min->value;
			if(!interval.min->is_inclusive) {
				++value;
			}

			this->field("minLength") << value;
		}

		if(interval.max.has_value()) {
			auto value = interval.max->value;
			if(!interval.max->is_inclusive) {
				--value;
			}

			this->field("maxLength") << value;
		}
	}

	std::string toString(Type t) {
		switch(t) {
		case Type::Nil: return "null";
		case Type::Bool: return "boolean";
		case Type::Int: return "integer";
		case Type::Num: return "number";
		case Type::Str: return "string";
		case Type::Map: return "object";
		case Type::List: return "array";

		default: throw InvalidTypeError(t);
		}
	}

	void tab() const {
		if(use_tabs) {
			this->dst << std::string(this->scopes.size(), '\t');
		} else {
			this->dst << std::string(this->tab_size * this->scopes.size(), ' ');
		}
	}
	void scope(std::array<char, 2> bracket, std::function<void()> const& functor) {
		this->dst << bracket[0] << std::endl;
		this->scopes.emplace_back(0);
		this->tab();

		functor();

		this->dst << std::endl;
		this->scopes.pop_back();
		this->tab();
		this->dst << bracket[1];
	}

	void scopeO(std::function<void()> const& functor) {
		this->scope({'{', '}'}, functor);
	}

	void scopeA(std::function<void()> const& functor) {
		this->scope({'[', ']'}, functor);
	}

	void enter() {
		bool const is_first = this->scopes.back()++ == 0;
		if(is_first) {
			return;
		}

		this->dst << "," << std::endl;
		this->tab();
	}

	std::ostream& field(std::string const& key) {
		this->enter();
		this->dst << std::quoted(key) << ": ";

		return this->dst;
	}

	std::ostream& value() {
		this->enter();
		return this->dst;
	}

	void fieldO(std::string const& key, std::function<void()> functor) {
		this->field(key);
		this->scopeO(functor);
	}

	void fieldA(std::string const& key, std::function<void()> functor) {
		this->field(key);
		this->scopeA(functor);
	}

	std::ostream& dst;

	bool        use_tabs = true;
	std::size_t tab_size = 4;

	std::vector<std::size_t> scopes;
};

class JsonSchemaReporter: public Reporter {
   protected:
	void report_(std::ostream& dst, detail::Prop const& prop) const override {
		auto ctx = detail::ReportContext{.dst = dst};

		dst << std::boolalpha;

		ctx.scopeO([&] {
			ctx.report(prop);
		});
	}
};

}  // namespace
}  // namespace detail

namespace report {

void asJsonSchema(std::ostream& dst, Node node) {
	detail::JsonSchemaReporter{}.report(dst, node);
}

}  // namespace report

}  // namespace cray

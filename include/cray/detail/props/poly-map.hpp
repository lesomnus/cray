#pragma once

#include <memory>
#include <string>

#include "cray/detail/ordered_map.hpp"
#include "cray/detail/ordered_set.hpp"
#include "cray/detail/prop.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

class PolyMpaProp: public Prop {
   public:
	using Prop::Prop;

	Type type() const override {
		return Type::Map;
	}

	std::string name() const override {
		return "Map";
	}

	bool ok() const override {
		for(auto const& [_, prop]: this->next_props) {
			if(!prop->ok()) {
				return false;
			}
		}

		return true;
	}

	bool hasDefault() const {
		return false;
	}

	void markRequired(Reference const& ref) override {
		required_keys.insert(ref.key());
	}

	bool needs(Reference const& ref) const override {
		return this->required_keys.end() != this->required_keys.find(ref.key());
	}

	void assign(Reference const& ref, std::shared_ptr<Prop> prop) override {
		prop->source = this->source->next(ref);
		this->next_props.insert_or_assign(ref.key(), std::move(prop));
	}

	std::shared_ptr<Prop> at(Reference const& ref) const override {
		auto const it = this->next_props.find(ref.key());
		if(it == this->next_props.cend()) {
			return nullptr;
		} else {
			return it->second;
		}
	}

	OrderedMap<std::string, std::shared_ptr<Prop>> next_props;
	OrderedSet<std::string>                        required_keys;
};

}  // namespace detail
}  // namespace cray

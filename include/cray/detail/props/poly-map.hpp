#pragma once

#include <concepts>
#include <memory>
#include <string>

#include "cray/detail/ordered_map.hpp"
#include "cray/detail/ordered_set.hpp"
#include "cray/detail/prop.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

template<std::derived_from<Prop> Next>
class BasicPolyMapProp
    : public virtual Prop
    , public KeyedPropHolder {
   public:
	using NextPropType = Next;

	using Prop::Prop;

	Type type() const override {
		return Type::Map;
	}

	bool ok() const override {
		if(!this->source->is(Type::Map)) {
			return !this->isNeeded() || this->hasDefault();
		}

		for(auto const& [key, next_prop]: this->next_props) {
			next_prop->source = this->source->next(key);
			if(next_prop->ok()) {
				continue;
			}

			return false;
		}

		return true;
	}

	void assign(Reference const& ref, std::shared_ptr<Prop> prop) override {
		std::shared_ptr<NextPropType> next_prop;
		if constexpr(std::is_same_v<NextPropType, Prop>) {
			next_prop = std::move(prop);
		} else {
			next_prop = std::dynamic_pointer_cast<NextPropType>(std::move(prop));
			if(next_prop == nullptr) {
				throw InvalidAccessError();
			}
		}

		auto const& key = ref.key();

		if(this->source) {
			// `this` does not have source if it is created by
			// function such as `prop` or `field`.
			next_prop->source = this->source->next(key);
		}

		this->next_props.insert_or_assign(key, std::move(next_prop));
	}

	std::shared_ptr<Prop> at(Reference const& ref) const override {
		auto const it = this->next_props.find(ref.key());
		if(it == this->next_props.cend()) {
			return nullptr;
		} else {
			return it->second;
		}
	}

	bool isMono() const override {
		return false;
	}

	Interval<std::size_t> interval() const override {
		return Interval<std::size_t>::All();
	}

	void forEachProps(Source const& source, std::function<void(std::string const&, std::shared_ptr<Prop> const&)> const& functor) const override {
		for(auto const& [key, next_prop]: this->next_props) {
			next_prop->source = source.next(key);
			next_prop->ref    = key;

			functor(key, next_prop);
		}
	}

	OrderedMap<std::string, std::shared_ptr<NextPropType>> next_props;
};

class PolyMpaProp: public BasicPolyMapProp<Prop> {
   public:
	using BasicPolyMapProp<Prop>::BasicPolyMapProp;

	std::string name() const override {
		return "Map";
	}

	bool hasDefault() const {
		return false;
	}

	void encodeDefaultValueInto(Source& dst) const override {
	}

	void markRequired(Reference const& ref) override {
		required_keys.insert(ref.key());
	}
};

}  // namespace detail
}  // namespace cray

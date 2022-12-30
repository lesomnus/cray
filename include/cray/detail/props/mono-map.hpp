#pragma once

#include <algorithm>
#include <concepts>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include "cray/detail/ordered_set.hpp"
#include "cray/detail/prop.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

template<typename V, std::derived_from<CodecProp<V>> P>
class MonoMapProp
    : public CodecProp<std::unordered_map<std::string, V>>
    , public KeyedPropHolder {
   public:
	using StorageType     = std::unordered_map<std::string, V>;
	using NextPropType    = P;
	using NextStorageType = V;

	template<typename Ctx, bool = true>
	class DescriberBase: public detail::Describer<MonoMapProp<V, P>> {
	   public:
		using detail::Describer<MonoMapProp<V, P>>::Describer;
	};

	template<bool Dummy>
	class DescriberBase<GettableContext, Dummy>: public detail::Describer<MonoMapProp<V, P>> {
	   public:
		using detail::Describer<MonoMapProp<V, P>>::Describer;

		inline std::optional<StorageType> opt() const {
			return this->prop_->opt();
		}

		inline StorageType get() const {
			return this->prop_->get();
		}
	};

	template<typename Ctx>
	class Describer: public DescriberBase<Ctx> {
	   public:
		using ContextType = Ctx;

		using DescriberBase<Ctx>::DescriberBase;

		inline Describer const& defaultValue(StorageType value) const {
			this->prop_->default_value = std::move(value);
			return *this;
		}

		inline auto withDefault(StorageType value) const {
			this->defaultValue(std::move(value));
			if constexpr(std::is_same_v<Ctx, GettableContext>) {
				return this->prop_->get();
			} else {
				return *this;
			}
		}

		inline Describer const& containing(OrderedSet<std::string> keys) const {
			this->prop_->required_keys = std::move(keys);
			return *this;
		}

		inline auto operator||(StorageType value) const {
			return this->withDefault(std::move(value));
		}
	};

	using CodecProp<std::unordered_map<std::string, V>>::CodecProp;

	Type type() const override {
		return Type::Map;
	}

	std::string name() const override {
		return "Map of " + this->next_prop->name();
	}

	bool ok() const override {
		if(!this->source->is(Type::Map)) {
			return !this->isNeeded() || this->hasDefault();
		}

		if(!std::ranges::all_of(this->required_keys, HeldBy(*this->source))) {
			return false;
		}

		return true;
	}

	void assign(Reference const& ref, std::shared_ptr<Prop> prop) override {
		auto next = std::dynamic_pointer_cast<CodecProp<NextStorageType>>(std::move(prop));
		if(next == nullptr) {
			throw InvalidAccessError();
		}

		this->next_prop = std::move(next);
	}

	std::shared_ptr<Prop> at(Reference const& ref) const override {
		return this->next_prop;
	}

	bool isConcrete() const override {
		return false;
	}

	void forEachProps(Source const& source, std::function<void(std::string const&, std::shared_ptr<Prop> const&)> const& functor) const override {
		this->source->keys([&](std::string const& key) {
			this->next_prop->source = source.next(key);
			this->next_prop->ref    = key;

			functor(key, this->next_prop);
			return true;
		});
	}

	std::shared_ptr<CodecProp<NextStorageType>> next_prop;

   protected:
	void encodeInto_(Source& dst, StorageType const& value) const {
		for(auto const& [key, next_value]: value) {
			auto next = dst.next(key);
			this->next_prop->encodeInto(*next, next_value);
		}
	}

	bool decodeFrom_(Source const& src, StorageType& value) const {
		if(!src.is(Type::Map)) {
			return false;
		}

		if(!std::ranges::all_of(this->required_keys, HeldBy(src))) {
			return false;
		}

		src.keys([&](std::string const& key) {
			auto next = src.next(key);
			return this->next_prop->decodeFrom(*next, value[key]);
		});

		return true;
	}
};

template<std::derived_from<Prop> P>
using MonoMapPropOf = MonoMapProp<typename P::StorageType, P>;

template<typename V>
struct PropFor_<std::unordered_map<std::string, V>> {
	using type = MonoMapPropOf<PropFor<V>>;
};

template<typename V, typename P>
struct IsMonoPropHolder_<MonoMapProp<V, P>>: std::true_type { };

}  // namespace detail
}  // namespace cray

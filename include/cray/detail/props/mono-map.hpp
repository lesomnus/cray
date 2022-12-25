#pragma once

#include <algorithm>
#include <concepts>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "cray/detail/prop.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

class MonoMapPropAccessor {
   public:
	virtual std::vector<std::string> const& requiredKeys() const = 0;
};

template<typename V, std::derived_from<CodecProp<V>> P>
class MonoMapProp
    : public CodecProp<std::unordered_map<std::string, V>>
    , public MonoMapPropAccessor {
   public:
	using StorageType      = std::unordered_map<std::string, V>;
	using ChildPropType    = P;
	using ChildStorageType = V;

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

		inline Describer const& containing(std::vector<std::string> keys) const {
			std::unordered_set<std::string> unique_keys;
			unique_keys.reserve(keys.size());

			auto cursor = keys.begin();
			for(auto const& key: keys) {
				auto [_, ok] = unique_keys.insert(key);
				if(!ok) {
					continue;
				}

				*cursor = key;
				++cursor;
			}

			keys.resize(std::distance(keys.begin(), cursor));

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
		return "Map<" + this->next->name() + ">";
	}

	bool ok() const override {
		if(!this->source->is(Type::Map)) {
			return !this->isNeeded() || this->hasDefault();
		}

		if(!this->required_keys.empty()) {
			std::unordered_set<std::string> required_keys(this->required_keys.begin(), this->required_keys.end());

			auto const keys = this->source->keys();
			for(auto const key: keys) {
				required_keys.erase(key);
				if(required_keys.empty()) {
					break;
				}
			}

			if(!required_keys.empty()) {
				return false;
			}
		}

		return true;
	}

	void markRequired(Reference const& ref) override {
		this->required_keys.push_back(ref.key());
	}

	bool needs(Reference const& ref) const override {
		auto it = std::find(this->required_keys.begin(), this->required_keys.end(), ref.key());
		return it != this->required_keys.end();
	}

	std::shared_ptr<Prop> at(Reference const& ref) const override {
		return this->next;
	}

	std::vector<std::string> const& requiredKeys() const override {
		return required_keys;
	}

	std::shared_ptr<CodecProp<ChildStorageType>> next;
	std::vector<std::string>                     required_keys;

   protected:
	void encodeTo_(Source& dst, StorageType const& value) const {
		for(auto const& [key, next_value]: value) {
			this->next->encodeTo(*this->source->next(key), next_value);
		}
	}

	bool decodeFrom_(Source const& src, StorageType& value) const {
		if(!this->source->is(Type::Map)) {
			return false;
		}

		auto const keys = src.keys();

		{
			auto required_keys = this->required_keys;
			auto cursor        = required_keys.begin();
			for(auto const& key: required_keys) {
				auto it = std::find(keys.begin(), keys.end(), key);
				if(it != keys.end()) {
					continue;
				}

				++cursor;
			}

			if(cursor != required_keys.end()) {
				return false;
			}
		}

		for(auto const& key: keys) {
			auto const ok = this->next->decodeFrom(*this->source->next(key), value[key]);
			if(!ok) {
				return false;
			}
		}

		return true;
	}
};

template<std::derived_from<Prop> P>
using MonoMapPropOf = MonoMapProp<typename P::StorageType, P>;

template<typename T>
struct PropFor_<std::unordered_map<std::string, T>> {
	using type = MonoMapPropOf<PropFor<T>>;
};

}  // namespace detail
}  // namespace cray

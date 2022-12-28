#pragma once

#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include "cray/detail/props/scalar.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

class BoolProp: public ScalarProp<Type::Bool> {
   public:
	template<typename Ctx>
	class DescriberBase;

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

		inline auto operator||(StorageType value) const {
			return this->withDefault(std::move(value));
		}
	};

	using ScalarProp<Type::Bool>::ScalarProp;

	std::string name() const override {
		return "Bool";
	}

	bool ok() const override {
		StorageType value;
		if(!this->source->get(value)) {
			return !this->isNeeded() || this->default_value.has_value();
		}

		return true;
	}

   protected:
	bool decodeFrom_(Source const& src, StorageType& value) const override {
		if(!src.get(value)) {
			return false;
		}

		return true;
	}
};

template<>
struct PropOf_<Type::Bool> {
	using type = BoolProp;
};

template<>
struct PropFor_<bool> {
	using type = BoolProp;
};

template<typename Ctx>
class BoolProp::DescriberBase: public detail::Describer<BoolProp> {
   public:
	using detail::Describer<BoolProp>::Describer;
};

template<>
class BoolProp::DescriberBase<GettableContext>: public detail::Describer<BoolProp> {
   public:
	using detail::Describer<BoolProp>::Describer;

	inline std::optional<StorageType> opt() const {
		return this->prop_->opt();
	}

	inline StorageType get() const {
		return this->prop_->get();
	}

	inline operator bool() const {
		return this->get();
	}
};

}  // namespace detail
}  // namespace cray

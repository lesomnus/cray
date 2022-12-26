#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <type_traits>

#include "cray/detail/props/scalar.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

class NilProp: public ScalarProp<Type::Nil> {
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

	using ScalarProp<Type::Nil>::ScalarProp;

	std::string name() const override {
		return "Nil";
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
struct PropOf_<Type::Nil> {
	using type = NilProp;
};

template<>
struct PropFor_<std::nullptr_t> {
	using type = NilProp;
};

template<typename Ctx>
class NilProp::DescriberBase: public detail::Describer<NilProp> {
   public:
	using detail::Describer<NilProp>::Describer;
};

template<>
class NilProp::DescriberBase<GettableContext>: public detail::Describer<NilProp> {
   public:
	using detail::Describer<NilProp>::Describer;

	inline std::optional<StorageType> opt() const {
		return this->prop_->opt();
	}

	inline StorageType get() const {
		return this->prop_->get();
	}

	inline operator std::nullptr_t() const {
		return nullptr;
	}
};

}  // namespace detail
}  // namespace cray

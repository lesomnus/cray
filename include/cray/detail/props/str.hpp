#pragma once

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>

#include "cray/detail/interval.hpp"
#include "cray/detail/props/scalar.hpp"
#include "cray/source.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

class StrProp: public ScalarProp<Type::Str> {
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

		inline Describer const& length(Interval<std::size_t> interval) const {
			this->prop_->length = interval;
			return *this;
		}

		inline auto operator||(StorageType value) const {
			return this->withDefault(std::move(value));
		}
	};

	using ScalarProp<Type::Str>::ScalarProp;

	std::string name() const override {
		return "String";
	}

	bool ok() const override {
		StorageType value;
		if(!this->source->get(value)) {
			return !this->isNeeded() || this->default_value.has_value();
		}

		if(!this->length.contains(value.length())) {
			return false;
		}

		return true;
	}

	Interval<std::size_t> length;

   protected:
	bool decodeFrom_(Source const& src, StorageType& value) const override {
		if(!src.get(value)) {
			return false;
		}

		if(!this->length.contains(value.length())) {
			return false;
		}

		return true;
	}
};

template<>
struct PropOf_<Type::Str> {
	using type = StrProp;
};

template<>
struct PropFor_<std::string> {
	using type = StrProp;
};

template<typename Ctx>
class StrProp::DescriberBase: public detail::Describer<StrProp> {
   public:
	using detail::Describer<StrProp>::Describer;
};

template<>
class StrProp::DescriberBase<GettableContext>: public detail::Describer<StrProp> {
   public:
	using detail::Describer<StrProp>::Describer;

	inline std::optional<StorageType> opt() const {
		return this->prop_->opt();
	}

	inline StorageType get() const {
		return this->prop_->get();
	}

	inline operator std::string() const {
		return this->get();
	}
};

}  // namespace detail
}  // namespace cray

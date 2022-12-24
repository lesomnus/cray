#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

#include "cray/detail/interval.hpp"
#include "cray/detail/props/scalar.hpp"
#include "cray/detail/tester.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

class IntProp: public ScalarProp<Type::Int> {
   public:
	template<typename Ctx>
	class DescriberBase;

	template<typename Ctx>
	class Describer: public DescriberBase<Ctx> {
	   public:
		using ContextType = Ctx;

		using DescriberBase<Ctx>::DescriberBase;

		inline Describer const& defaultValue(StorageType const value) const {
			this->prop_->default_value = value;
			return *this;
		}

		inline auto withDefault(StorageType value) const {
			this->defaultValue(value);
			if constexpr(std::is_same_v<Ctx, GettableContext>) {
				return this->prop_->get();
			} else {
				return *this;
			}
		}

		inline Describer const& mutipleOf(StorageType const value) const {
			if(value == 0) [[unlikely]] {
				throw std::invalid_argument("value cannot be multiple of zero");
			}

			this->prop_->multiple_of.divisor = value;
			return *this;
		}

		inline Describer const& interval(Interval<StorageType> interval) const {
			this->prop_->interval = interval;
			return *this;
		}

		inline Describer const& withClamp() const {
			this->prop_->with_clamp = true;
			return *this;
		}

		inline auto operator||(StorageType value) const {
			return this->withDefault(value);
		}
	};

	using ScalarProp<Type::Int>::ScalarProp;

	std::string name() const override {
		return "Integer";
	}

	bool ok() const override {
		StorageType value;
		if(!this->source->get(value)) {
			return !this->isNeeded() || this->default_value.has_value();
		}

		if(!this->multiple_of(value)) {
			return false;
		}

		if(!this->interval.contains(value) && !this->with_clamp) {
			return false;
		}

		return true;
	}

	DivisibilityTest<StorageType> multiple_of;
	Interval<StorageType>         interval;
	bool                          with_clamp = false;

   protected:
	bool decodeFrom_(Source const& src, StorageType& value) const override {
		if(!src.get(value)) {
			return false;
		}

		if(!this->multiple_of(value)) {
			return false;
		}

		if(!this->interval.contains(value)) {
			if(!this->with_clamp) {
				return false;
			}

			value = this->interval.clamp(value);
		}

		return true;
	}
};

template<>
struct PropOf_<Type::Int> {
	using type = IntProp;
};

template<std::integral T>
struct PropFor_<T> {
	using type = IntProp;
};

template<typename Ctx>
class IntProp::DescriberBase: public detail::Describer<IntProp> {
   public:
	using detail::Describer<IntProp>::Describer;
};

template<>
class IntProp::DescriberBase<GettableContext>: public detail::Describer<IntProp> {
   public:
	using detail::Describer<IntProp>::Describer;

	inline std::optional<StorageType> opt() const {
		return this->prop_->opt();
	}

	inline StorageType get() const {
		return this->prop_->get();
	}

	// clang-format off
	inline operator short()     const { return this->get_<short    >(); }
	inline operator int()       const { return this->get_<int      >(); }
	inline operator long()      const { return this->get_<long     >(); }
	inline operator long long() const { return this->get_<long long>(); }

	inline operator unsigned short()     const { return this->get_<unsigned short    >(); }
	inline operator unsigned()           const { return this->get_<unsigned          >(); }
	inline operator unsigned long()      const { return this->get_<unsigned long     >(); }
	inline operator unsigned long long() const { return this->get_<unsigned long long>(); }
	// clang-format on

   private:
	template<typename T>
	inline T get_() const {
		return static_cast<T>(this->prop_->get());
	}
};

}  // namespace detail
}  // namespace cray

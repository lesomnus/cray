#pragma once

#include <cassert>
#include <cmath>
#include <concepts>
#include <limits>

#include "cray/detail/interval.hpp"
#include "cray/detail/props/scalar.hpp"
#include "cray/source.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

template<typename T>
struct DivisibilityTest {
	constexpr bool empty() const {
		return this->divisor == 0;
	}

	constexpr bool operator()(T value) const {
		if(this->empty()) {
			return true;
		}

		if constexpr(std::is_integral_v<T>) {
			return (value % this->divisor) == 0;
		} else {
			return std::abs(std::fmod(value, this->divisor)) < std::numeric_limits<T>::epsilon();
		}
	}

	T divisor = 0;
};

template<Type T>
class NumericProp: public ScalarProp<T> {
   public:
	using StorageType = StorageOf<T>;

	template<typename Ctx, template<typename> typename Base>
	class Describer: public Base<Ctx> {
	   public:
		using ContextType = Ctx;

		using Base<Ctx>::Base;

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

	using ScalarProp<T>::ScalarProp;

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

}  // namespace detail
}  // namespace cray

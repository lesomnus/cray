#pragma once

#include <cmath>
#include <concepts>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "cray/detail/interval.hpp"
#include "cray/detail/prop.hpp"
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

	template<typename Ctx, template<typename, bool> typename Base>
	class Describer: public Base<Ctx, true> {
	   public:
		using ContextType = Ctx;

		using Base<Ctx, true>::Base;

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

	void encodeDefaultValueInto(Source& dst) const override {
		this->encodeInto(dst, this->default_value.value());
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

template<typename V>
class BasicNumericProp
    : public CodecProp<V>
    , public NumericProp<TypeFor<V>> {
   public:
	using NumericProp<TypeFor<V>>::NumericProp;

	std::string name() const override {
		return NumericProp<TypeFor<V>>::name();
	}

	bool hasDefault() const override {
		return NumericProp<TypeFor<V>>::hasDefault();
	}

	void encodeDefaultValueInto(Source& dst) const override {
		NumericProp<TypeFor<V>>::encodeDefaultValueInto(dst);
	}

	using CodecProp<V>::opt;
	using CodecProp<V>::get;

   protected:
	void encodeInto_(Source& dst, V const& value) const override {
		auto const v = static_cast<StorageOf<TypeFor<V>>>(value);

		dst.set(v);
	}

	bool decodeFrom_(Source const& src, V& value) const override {
		StorageOf<TypeFor<V>> v;

		bool const ok = NumericProp<TypeFor<V>>::decodeFrom_(src, v);

		value = static_cast<V>(v);
		return ok;
	}
};

template<>
class BasicNumericProp<StorageOf<Type::Int>>: public NumericProp<Type::Int> {
   public:
	using NumericProp<Type::Int>::NumericProp;
};

template<>
class BasicNumericProp<StorageOf<Type::Num>>: public NumericProp<Type::Num> {
   public:
	using NumericProp<Type::Num>::NumericProp;
};

}  // namespace detail
}  // namespace cray

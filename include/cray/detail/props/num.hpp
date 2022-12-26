#pragma once

#include <concepts>
#include <optional>
#include <string>

#include "cray/detail/props/numeric.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

class NumProp: public NumericProp<Type::Num> {
   public:
	template<typename Ctx>
	class DescriberBase;

	template<typename Ctx>
	using Describer = NumericProp<Type::Num>::Describer<Ctx, DescriberBase>;

	using NumericProp<Type::Num>::NumericProp;

	std::string name() const override {
		return "Number";
	}
};

template<>
struct PropOf_<Type::Num> {
	using type = NumProp;
};

template<std::floating_point T>
struct PropFor_<T> {
	using type = NumProp;
};

template<typename Ctx>
class NumProp::DescriberBase: public detail::Describer<NumProp> {
   public:
	using detail::Describer<NumProp>::Describer;
};

template<>
class NumProp::DescriberBase<GettableContext>: public detail::Describer<NumProp> {
   public:
	using detail::Describer<NumProp>::Describer;

	inline std::optional<StorageType> opt() const {
		return this->prop_->opt();
	}

	inline StorageType get() const {
		return this->prop_->get();
	}

	// clang-format off
	inline operator float()       const { return this->get_<float      >(); }
	inline operator double()      const { return this->get_<double     >(); }
	inline operator long double() const { return this->get_<long double>(); }
	// clang-format on

   private:
	template<typename T>
	inline T get_() const {
		return static_cast<T>(this->prop_->get());
	}
};

}  // namespace detail
}  // namespace cray

#pragma once

#include <concepts>
#include <optional>
#include <string>

#include "cray/detail/props/numeric.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

template<std::floating_point V>
class BasicNumProp: public BasicNumericProp<V> {
   public:
	using StorageType = V;

	template<typename Ctx, bool = true>
	class DescriberBase: public detail::Describer<BasicNumProp<V>> {
	   public:
		using detail::Describer<BasicNumProp<V>>::Describer;
	};

	template<bool Dummy>
	class DescriberBase<GettableContext, Dummy>: public detail::Describer<BasicNumProp<V>> {
	   public:
		using detail::Describer<BasicNumProp<V>>::Describer;

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
		template<typename V_>
		inline V_ get_() const {
			return static_cast<V_>(this->prop_->get());
		}
	};

	template<typename Ctx>
	using Describer = NumericProp<Type::Num>::Describer<Ctx, DescriberBase>;

	using BasicNumericProp<V>::BasicNumericProp;

	std::string name() const override {
		return "Number";
	}
};

using NumProp = BasicNumProp<StorageOf<Type::Num>>;

template<>
struct PropOf_<Type::Num> {
	using type = NumProp;
};

template<std::floating_point V>
struct PropFor_<V> {
	using type = BasicNumProp<V>;
};

}  // namespace detail
}  // namespace cray

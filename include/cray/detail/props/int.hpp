#pragma once

#include <concepts>
#include <optional>
#include <string>

#include "cray/detail/props/numeric.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

template<std::integral V>
class BasicIntProp: public BasicNumericProp<V> {
   public:
	using StorageType = V;

	template<typename Ctx, bool = true>
	class DescriberBase: public detail::Describer<BasicIntProp<V>> {
	   public:
		using detail::Describer<BasicIntProp<V>>::Describer;
	};

	template<bool Dummy>
	class DescriberBase<GettableContext, Dummy>: public detail::Describer<BasicIntProp<V>> {
	   public:
		using detail::Describer<BasicIntProp<V>>::Describer;

		inline std::optional<StorageType> opt() const {
			return this->prop_->opt();
		}

		inline StorageType get() const {
			return this->prop_->get();
		}

		template<std::integral V_>
		inline operator V_() const { return this->get_<V_>(); }

	   private:
		template<typename V_>
		inline V_ get_() const {
			return static_cast<V_>(this->prop_->get());
		}
	};

	template<typename Ctx>
	using Describer = NumericProp<Type::Int>::Describer<Ctx, DescriberBase>;

	using BasicNumericProp<V>::BasicNumericProp;

	std::string name() const override {
		return "Integer";
	}
};

using IntProp = BasicIntProp<StorageOf<Type::Int>>;

template<>
struct PropOf_<Type::Int> {
	using type = IntProp;
};

template<std::integral V>
struct PropFor_<V> {
	using type = BasicIntProp<V>;
};

}  // namespace detail
}  // namespace cray

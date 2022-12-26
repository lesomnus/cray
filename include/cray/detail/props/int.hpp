#pragma once

#include <concepts>
#include <optional>
#include <string>

#include "cray/detail/props/numeric.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

class IntProp: public NumericProp<Type::Int> {
   public:
	template<typename Ctx>
	class DescriberBase;

	template<typename Ctx>
	using Describer = NumericProp<Type::Int>::Describer<Ctx, DescriberBase>;

	using NumericProp<Type::Int>::NumericProp;

	std::string name() const override {
		return "Integer";
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

#pragma once

#include "cray/detail/prop.hpp"

namespace cray {
namespace detail {

template<Type T>
class ScalarProp: public CodecProp<StorageOf<T>> {
   public:
	using StorageType = StorageOf<T>;

	using CodecProp<StorageOf<T>>::CodecProp;

	Type type() const override {
		return T;
	}

   protected:
	void encodeInto_(Source& dst, StorageType const& value) const override {
		dst.set(value);
	}
};

}  // namespace detail
}  // namespace cray

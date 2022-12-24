#pragma once

#include <cassert>
#include <concepts>

namespace cray {
namespace detail {

template<std::integral T>
struct DivisibilityTest {
	constexpr bool operator()(T value) const {
		if(this->divisor == 0) {
			return true;
		}

		return (value % this->divisor) == 0;
	}

	T divisor = 0;
};

}  // namespace detail
}  // namespace cray

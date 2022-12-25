#pragma once

#include <cassert>
#include <concepts>

namespace cray {
namespace detail {

template<std::integral T>
struct DivisibilityTest {
	constexpr bool empty() const {
		return this->divisor == 0;
	}

	constexpr bool operator()(T value) const {
		if(this->empty()) {
			return true;
		}

		return (value % this->divisor) == 0;
	}

	T divisor = 0;
};

}  // namespace detail
}  // namespace cray

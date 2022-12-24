#pragma once

#include <cmath>
#include <concepts>
#include <limits>
#include <optional>
#include <type_traits>

namespace cray {
namespace detail {

template<typename T, typename U>
concept SameDomainWith = (std::is_integral_v<T> ? std::is_integral_v<U> : std::is_floating_point_v<U>);

template<typename T>
struct Bound {
	T    value;
	bool is_inclusive;
};

template<typename T>
struct LowerBound: Bound<T> {
	template<typename U>
	constexpr bool contains(U value) const noexcept {
		if constexpr(std::is_unsigned_v<T>) {
			if(value < 0) {
				return false;
			}
		}

		if(this->is_inclusive) {
			return this->value <= value;
		} else {
			return this->value < value;
		}
	}

	template<typename U>
	constexpr std::common_type_t<T, U> clamp(U value) const noexcept {
		using V = std::common_type_t<T, U>;

		if constexpr(std::is_unsigned_v<T>) {
			if(value < 0) {
				if(this->is_inclusive) {
					return this->value;
				} else {
					return this->value + 1;
				}
			}
		}

		if(this->value < value) {
			return value;
		} else if(this->is_inclusive) {
			return this->value;
		} else if constexpr(std::is_integral_v<T>) {
			return static_cast<V>(this->value) + 1;
		} else {
			return std::nextafter(static_cast<V>(this->value), std::numeric_limits<V>::max());
		}
	}

	template<typename U>
	constexpr bool operator==(const LowerBound<U>& other) const noexcept {
		return (this->value == other.value) && (this->is_inclusive == other.is_inclusive);
	}

	template<typename U>
	constexpr auto operator<=>(const LowerBound<U>& other) const noexcept {
		using Ordering = std::conditional_t<std::is_integral_v<std::common_type_t<T, U>>, std::strong_ordering, std::partial_ordering>;

		const Ordering res = this->value <=> other.value;
		if(res != 0) {
			return res;
		} else if(this->is_inclusive == other.is_inclusive) {
			return Ordering::equivalent;
		} else if(this->is_inclusive) {
			return Ordering::less;
		} else {
			return Ordering::greater;
		}
	}

	template<typename U>
	constexpr operator LowerBound<U>() const noexcept {
		return LowerBound<U>{static_cast<U>(this->value), this->is_inclusive};
	}
};

template<typename T>
struct UpperBound: Bound<T> {
	template<typename U>
	constexpr bool contains(U value) const noexcept {
		if constexpr(std::is_unsigned_v<T>) {
			if(value < 0) {
				return true;
			}
		}

		if(this->is_inclusive) {
			return this->value >= value;
		} else {
			return this->value > value;
		}
	}

	template<typename U>
	constexpr std::common_type_t<T, U> clamp(U value) const noexcept {
		using V = std::common_type_t<T, U>;

		if constexpr(std::is_unsigned_v<T>) {
			if(value < 0) {
				return value;
			}
		}

		if(this->value > value) {
			return value;
		} else if(this->is_inclusive) {
			return this->value;
		} else if constexpr(std::is_integral_v<T>) {
			return static_cast<V>(this->value) - 1;
		} else {
			return std::nextafter(static_cast<V>(this->value), std::numeric_limits<V>::lowest());
		}
	}

	template<typename U>
	constexpr bool operator==(const UpperBound<U>& other) const noexcept {
		return (this->value == other.value) && (this->is_inclusive == other.is_inclusive);
	}

	template<typename U>
	constexpr auto operator<=>(const UpperBound<U>& other) const noexcept {
		using Ordering = std::conditional_t<std::is_integral_v<std::common_type_t<T, U>>, std::strong_ordering, std::partial_ordering>;

		const Ordering res = this->value <=> other.value;
		if(res != 0) {
			return res;
		} else if(this->is_inclusive == other.is_inclusive) {
			return Ordering::equivalent;
		} else if(this->is_inclusive) {
			return Ordering::greater;
		} else {
			return Ordering::less;
		}
	}

	template<typename U>
	constexpr operator UpperBound<U>() const noexcept {
		return UpperBound<U>{static_cast<U>(this->value), this->is_inclusive};
	}
};

template<typename T, typename U>
constexpr bool operator<(const UpperBound<T>& lhs, const LowerBound<T>& rhs) noexcept {
	const auto res = lhs.value <=> rhs.value;
	if(res != 0) {
		return res < 0;
	} else {
		return !(lhs.is_inclusive && rhs.is_inclusive);
	}
}

template<typename T>
struct Interval {
	using ValueType      = T;
	using LowerBoundType = LowerBound<ValueType>;
	using UpperBoundType = UpperBound<ValueType>;

	static constexpr Interval Empty() noexcept {
		return Interval{
		    .min = LowerBoundType{+2, false},
		    .max = UpperBoundType{+1, false},
		};
	}

	template<SameDomainWith<T> U>
	static constexpr Interval<std::common_type_t<T, U>> of() noexcept {
		using V = std::common_type_t<T, U>;
		return Interval<V>{
		    .min = LowerBound<V>{std::numeric_limits<U>::lowest(), true},
		    .max = UpperBound<V>{std::numeric_limits<U>::max(), true},
		};
	}

	constexpr bool empty() const {
		if(!(this->min && this->max)) {
			return false;
		}

		if(this->min->value < this->max->value) {
			return false;
		}

		if((this->min->value == this->max->value)) {
			return !(this->min->is_inclusive && this->max->is_inclusive);
		}

		return true;
	}

	template<typename U>
	constexpr bool contains(const U& value) const {
		if(this->min && (!this->min->contains(value))) {
			return false;
		}
		if(this->max && (!this->max->contains(value))) {
			return false;
		}

		return true;
	}

	template<typename U>
	constexpr std::common_type_t<T, U> clamp(U value) const {
		std::common_type_t<T, U> v = value;

		if(this->min) {
			v = this->min->clamp(v);
		}
		if(this->max) {
			v = this->max->clamp(v);
		}

		return v;
	}

	constexpr Interval intersect(const Interval& other) const noexcept {
		if(this->empty() || other.empty()) {
			return Interval::Empty();
		}

		Interval const* lhs = this;
		Interval const* rhs = &other;

		// Sort intervals.
		// Interval that has greater max be `rhs`.
		if(!lhs->max || (rhs->max && (*rhs->max < *lhs->max))) {
			std::swap(lhs, rhs);
		}

		if(Interval{rhs->min, lhs->max}.empty()) {
			return Interval::Empty();
		}

		Interval rst = *lhs;
		if(!lhs->min || (rhs->min && (*lhs->min < *rhs->min))) {
			// Set greater min.
			rst.min = rhs->min;
		}

		return rst;
	}

	constexpr bool operator==(Interval const& other) const noexcept {
		return (this->min == other.min) && (this->max == other.max);
	}

	template<typename U>
	constexpr auto operator=(ValueType value) const noexcept {
		using V = std::common_type_t<T, U>;
		return Interval<T>{LowerBound<V>{static_cast<V>(value), true}, UpperBound<V>{static_cast<V>(value), true}};
	}

	template<typename U>
	constexpr auto operator<(U value) const noexcept {
		using V = std::common_type_t<T, U>;
		return Interval<V>{this->min, UpperBound<V>{static_cast<V>(value), false}};
	}

	template<typename U>
	constexpr auto operator<=(U value) const noexcept {
		using V = std::common_type_t<T, U>;
		return Interval<V>{this->min, UpperBound<V>{static_cast<V>(value), true}};
	}

	template<typename U>
	constexpr operator Interval<U>() const noexcept {
		return Interval<U>{this->min, this->max};
	}

	std::optional<LowerBoundType> min;
	std::optional<UpperBoundType> max;
};

struct IntervalPlaceHolder {
	template<typename T>
	constexpr Interval<T> operator<(T value) const noexcept {
		return Interval<T>{std::nullopt, UpperBound<T>{value, false}};
	}

	template<typename T>
	constexpr Interval<T> operator<=(T value) const noexcept {
		return Interval<T>{std::nullopt, UpperBound<T>{value, true}};
	}
};

}  // namespace detail

static constexpr detail::IntervalPlaceHolder x{};

}  // namespace cray

template<typename T>
constexpr cray::detail::Interval<T> operator<(T value, cray::detail::IntervalPlaceHolder) noexcept {
	return cray::detail::Interval<T>{cray::detail::LowerBound<T>{value, false}, std::nullopt};
}

template<typename T>
constexpr cray::detail::Interval<T> operator<=(T value, cray::detail::IntervalPlaceHolder) noexcept {
	return cray::detail::Interval<T>{cray::detail::LowerBound<T>{value, true}, std::nullopt};
}

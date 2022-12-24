#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cray/detail/interval.hpp>

template<typename T>
using Lim = std::numeric_limits<T>;

constexpr std::int64_t I64Max = Lim<std::int64_t>::max();
constexpr std::int64_t I64Min = Lim<std::int64_t>::min();
constexpr std::int32_t I32Max = Lim<std::int32_t>::max();
constexpr std::int32_t I32Min = Lim<std::int32_t>::min();

constexpr std::uint64_t U64Max = Lim<std::uint64_t>::max();
constexpr std::uint64_t U64Min = Lim<std::uint64_t>::min();
constexpr std::uint32_t U32Max = Lim<std::uint32_t>::max();
constexpr std::uint32_t U32Min = Lim<std::uint32_t>::min();

constexpr double F64Max = Lim<double>::max();
constexpr double F64Min = Lim<double>::lowest();
constexpr float  F32Max = Lim<float>::max();
constexpr float  F32Min = Lim<float>::lowest();

const double next0d = std::nextafter(0., 1.);
const double prev0d = std::nextafter(0., -1.);
const float  next0f = std::nextafter(0.f, 1.f);
const float  prev0f = std::nextafter(0.f, -1.f);

using NumberTypes = std::tuple<
    double,
    float,
    std::uint64_t,
    std::int64_t,
    std::uint32_t,
    std::int32_t>;

using NumberTypePairs = std::tuple<
    std::pair<std::uint32_t, std::uint32_t>,
    std::pair<std::int32_t, std::uint32_t>,
    std::pair<std::uint32_t, std::int32_t>,

    std::pair<std::int32_t, std::int32_t>,
    std::pair<std::int32_t, std::int64_t>,
    std::pair<std::int64_t, std::int32_t>,
    std::pair<std::int64_t, std::int64_t>,

    std::pair<std::int32_t, float>,
    std::pair<float, std::int32_t>,

    std::pair<float, float>,
    std::pair<float, double>,
    std::pair<double, double>>;

TEST_CASE("SameDomainWith") {
	using cray::detail::SameDomainWith;

	STATIC_REQUIRE(SameDomainWith<std::uint32_t, std::uint32_t>);
	STATIC_REQUIRE(SameDomainWith<std::int32_t, std::uint32_t>);
	STATIC_REQUIRE(SameDomainWith<std::uint32_t, std::int32_t>);

	STATIC_REQUIRE(SameDomainWith<std::int32_t, std::int32_t>);
	STATIC_REQUIRE(SameDomainWith<std::int32_t, std::int64_t>);
	STATIC_REQUIRE(SameDomainWith<std::int64_t, std::int32_t>);
	STATIC_REQUIRE(SameDomainWith<std::int64_t, std::int64_t>);

	STATIC_REQUIRE(not SameDomainWith<std::int32_t, float>);
	STATIC_REQUIRE(not SameDomainWith<float, std::int32_t>);

	STATIC_REQUIRE(SameDomainWith<float, float>);
	STATIC_REQUIRE(SameDomainWith<float, double>);
	STATIC_REQUIRE(SameDomainWith<double, float>);
	STATIC_REQUIRE(SameDomainWith<double, double>);
}

TEMPLATE_LIST_TEST_CASE("Bound::contains-onZero", "", NumberTypes) {
	using LB = cray::detail::LowerBound<TestType>;
	using UB = cray::detail::UpperBound<TestType>;

	constexpr LB gte0{0, true};
	STATIC_REQUIRE(gte0.contains(F64Max));
	STATIC_REQUIRE(gte0.contains(F32Max));
	STATIC_REQUIRE(gte0.contains(U64Max));
	STATIC_REQUIRE(gte0.contains(I64Max));
	STATIC_REQUIRE(gte0.contains(U32Max));
	STATIC_REQUIRE(gte0.contains(I32Max));
	STATIC_REQUIRE(gte0.contains(1));
	REQUIRE(gte0.contains(next0f));
	REQUIRE(gte0.contains(next0d));
	STATIC_REQUIRE(gte0.contains(0));
	REQUIRE(!gte0.contains(prev0d));
	REQUIRE(!gte0.contains(prev0f));
	STATIC_REQUIRE(!gte0.contains(-1));
	STATIC_REQUIRE(!gte0.contains(I32Min));
	STATIC_REQUIRE(!gte0.contains(I64Min));
	STATIC_REQUIRE(!gte0.contains(F32Min));
	STATIC_REQUIRE(!gte0.contains(F64Min));

	constexpr LB gt0{0, false};
	STATIC_REQUIRE(gt0.contains(F64Max));
	STATIC_REQUIRE(gt0.contains(F32Max));
	STATIC_REQUIRE(gt0.contains(U64Max));
	STATIC_REQUIRE(gt0.contains(I64Max));
	STATIC_REQUIRE(gt0.contains(U32Max));
	STATIC_REQUIRE(gt0.contains(I32Max));
	STATIC_REQUIRE(gt0.contains(1));
	REQUIRE(gt0.contains(next0f));
	REQUIRE(gt0.contains(next0d));
	STATIC_REQUIRE(!gt0.contains(0));
	REQUIRE(!gt0.contains(prev0d));
	REQUIRE(!gt0.contains(prev0f));
	STATIC_REQUIRE(!gt0.contains(-1));
	STATIC_REQUIRE(!gt0.contains(I32Min));
	STATIC_REQUIRE(!gt0.contains(I64Min));
	STATIC_REQUIRE(!gt0.contains(F32Min));
	STATIC_REQUIRE(!gt0.contains(F64Min));

	constexpr UB lt0{0, false};
	STATIC_REQUIRE(!lt0.contains(F64Max));
	STATIC_REQUIRE(!lt0.contains(F32Max));
	STATIC_REQUIRE(!lt0.contains(U64Max));
	STATIC_REQUIRE(!lt0.contains(I64Max));
	STATIC_REQUIRE(!lt0.contains(U32Max));
	STATIC_REQUIRE(!lt0.contains(I32Max));
	STATIC_REQUIRE(!lt0.contains(1));
	REQUIRE(!lt0.contains(next0f));
	REQUIRE(!lt0.contains(next0d));
	STATIC_REQUIRE(!lt0.contains(0));
	REQUIRE(lt0.contains(prev0d));
	REQUIRE(lt0.contains(prev0f));
	STATIC_REQUIRE(lt0.contains(-1));
	STATIC_REQUIRE(lt0.contains(I32Min));
	STATIC_REQUIRE(lt0.contains(I64Min));
	STATIC_REQUIRE(lt0.contains(F32Min));
	STATIC_REQUIRE(lt0.contains(F64Min));

	constexpr UB lte0{0, true};
	STATIC_REQUIRE(!lte0.contains(F64Max));
	STATIC_REQUIRE(!lte0.contains(F32Max));
	STATIC_REQUIRE(!lte0.contains(U64Max));
	STATIC_REQUIRE(!lte0.contains(I64Max));
	STATIC_REQUIRE(!lte0.contains(U32Max));
	STATIC_REQUIRE(!lte0.contains(I32Max));
	STATIC_REQUIRE(!lte0.contains(1));
	REQUIRE(!lte0.contains(next0f));
	REQUIRE(!lte0.contains(next0d));
	STATIC_REQUIRE(lte0.contains(0));
	REQUIRE(lte0.contains(prev0d));
	REQUIRE(lte0.contains(prev0f));
	STATIC_REQUIRE(lte0.contains(-1));
	STATIC_REQUIRE(lte0.contains(I32Min));
	STATIC_REQUIRE(lte0.contains(I64Min));
	STATIC_REQUIRE(lte0.contains(F32Min));
	STATIC_REQUIRE(lte0.contains(F64Min));
}

TEST_CASE("Bound::contains-onEdge") {
	SECTION("by i32") {
		using LB = cray::detail::LowerBound<std::int32_t>;
		using UB = cray::detail::UpperBound<std::int32_t>;

		SECTION("min") {
			constexpr LB gt_i32_min{I32Min, false};
			STATIC_REQUIRE(gt_i32_min.contains(F64Max));
			STATIC_REQUIRE(gt_i32_min.contains(F32Max));
			STATIC_REQUIRE(gt_i32_min.contains(U64Max));
			STATIC_REQUIRE(gt_i32_min.contains(I64Max));
			STATIC_REQUIRE(gt_i32_min.contains(U32Max));
			STATIC_REQUIRE(gt_i32_min.contains(I32Max));
			STATIC_REQUIRE(gt_i32_min.contains(1));
			STATIC_REQUIRE(gt_i32_min.contains(0));
			STATIC_REQUIRE(gt_i32_min.contains(-1));
			STATIC_REQUIRE(!gt_i32_min.contains(I32Min));
			STATIC_REQUIRE(!gt_i32_min.contains(I64Min));
			STATIC_REQUIRE(!gt_i32_min.contains(F32Min));
			STATIC_REQUIRE(!gt_i32_min.contains(F64Min));

			constexpr LB gte_i32_min{I32Min, true};
			STATIC_REQUIRE(gte_i32_min.contains(F64Max));
			STATIC_REQUIRE(gte_i32_min.contains(F32Max));
			STATIC_REQUIRE(gte_i32_min.contains(U64Max));
			STATIC_REQUIRE(gte_i32_min.contains(I64Max));
			STATIC_REQUIRE(gte_i32_min.contains(U32Max));
			STATIC_REQUIRE(gte_i32_min.contains(I32Max));
			STATIC_REQUIRE(gte_i32_min.contains(1));
			STATIC_REQUIRE(gte_i32_min.contains(0));
			STATIC_REQUIRE(gte_i32_min.contains(-1));
			STATIC_REQUIRE(gte_i32_min.contains(I32Min));
			STATIC_REQUIRE(!gte_i32_min.contains(I64Min));
			STATIC_REQUIRE(!gte_i32_min.contains(F32Min));
			STATIC_REQUIRE(!gte_i32_min.contains(F64Min));

			constexpr UB lte_i32_min{I32Min, true};
			STATIC_REQUIRE(!lte_i32_min.contains(F64Max));
			STATIC_REQUIRE(!lte_i32_min.contains(F32Max));
			STATIC_REQUIRE(!lte_i32_min.contains(U64Max));
			STATIC_REQUIRE(!lte_i32_min.contains(I64Max));
			STATIC_REQUIRE(!lte_i32_min.contains(U32Max));
			STATIC_REQUIRE(!lte_i32_min.contains(I32Max));
			STATIC_REQUIRE(!lte_i32_min.contains(1));
			STATIC_REQUIRE(!lte_i32_min.contains(0));
			STATIC_REQUIRE(!lte_i32_min.contains(-1));
			STATIC_REQUIRE(lte_i32_min.contains(I32Min));
			STATIC_REQUIRE(lte_i32_min.contains(I64Min));
			STATIC_REQUIRE(lte_i32_min.contains(F32Min));
			STATIC_REQUIRE(lte_i32_min.contains(F64Min));

			constexpr UB lt_i32_min{I32Min, false};
			STATIC_REQUIRE(!lt_i32_min.contains(F64Max));
			STATIC_REQUIRE(!lt_i32_min.contains(F32Max));
			STATIC_REQUIRE(!lt_i32_min.contains(U64Max));
			STATIC_REQUIRE(!lt_i32_min.contains(I64Max));
			STATIC_REQUIRE(!lt_i32_min.contains(U32Max));
			STATIC_REQUIRE(!lt_i32_min.contains(I32Max));
			STATIC_REQUIRE(!lt_i32_min.contains(1));
			STATIC_REQUIRE(!lt_i32_min.contains(0));
			STATIC_REQUIRE(!lt_i32_min.contains(-1));
			STATIC_REQUIRE(!lt_i32_min.contains(I32Min));
			STATIC_REQUIRE(lt_i32_min.contains(I64Min));
			STATIC_REQUIRE(lt_i32_min.contains(F32Min));
			STATIC_REQUIRE(lt_i32_min.contains(F64Min));
		}

		SECTION("max") {
			constexpr LB gt_i32_max{I32Max, false};
			STATIC_REQUIRE(gt_i32_max.contains(F64Max));
			STATIC_REQUIRE(gt_i32_max.contains(F32Max));
			STATIC_REQUIRE(gt_i32_max.contains(U64Max));
			STATIC_REQUIRE(gt_i32_max.contains(I64Max));
			STATIC_REQUIRE(gt_i32_max.contains(U32Max));
			STATIC_REQUIRE(!gt_i32_max.contains(I32Max));
			STATIC_REQUIRE(!gt_i32_max.contains(1));
			STATIC_REQUIRE(!gt_i32_max.contains(0));
			STATIC_REQUIRE(!gt_i32_max.contains(-1));
			STATIC_REQUIRE(!gt_i32_max.contains(I32Min));
			STATIC_REQUIRE(!gt_i32_max.contains(I64Min));
			STATIC_REQUIRE(!gt_i32_max.contains(F32Min));
			STATIC_REQUIRE(!gt_i32_max.contains(F64Min));

			constexpr LB gte_i32_max{I32Max, true};
			STATIC_REQUIRE(gte_i32_max.contains(F64Max));
			STATIC_REQUIRE(gte_i32_max.contains(F32Max));
			STATIC_REQUIRE(gte_i32_max.contains(U64Max));
			STATIC_REQUIRE(gte_i32_max.contains(I64Max));
			STATIC_REQUIRE(gte_i32_max.contains(U32Max));
			STATIC_REQUIRE(gte_i32_max.contains(I32Max));
			STATIC_REQUIRE(!gte_i32_max.contains(1));
			STATIC_REQUIRE(!gte_i32_max.contains(0));
			STATIC_REQUIRE(!gte_i32_max.contains(-1));
			STATIC_REQUIRE(!gte_i32_max.contains(I32Min));
			STATIC_REQUIRE(!gte_i32_max.contains(I64Min));
			STATIC_REQUIRE(!gte_i32_max.contains(F32Min));
			STATIC_REQUIRE(!gte_i32_max.contains(F64Min));

			constexpr UB lte_i32_max{I32Max, true};
			STATIC_REQUIRE(!lte_i32_max.contains(F64Max));
			STATIC_REQUIRE(!lte_i32_max.contains(F32Max));
			STATIC_REQUIRE(!lte_i32_max.contains(U64Max));
			STATIC_REQUIRE(!lte_i32_max.contains(I64Max));
			STATIC_REQUIRE(!lte_i32_max.contains(U32Max));
			STATIC_REQUIRE(lte_i32_max.contains(I32Max));
			STATIC_REQUIRE(lte_i32_max.contains(1));
			STATIC_REQUIRE(lte_i32_max.contains(0));
			STATIC_REQUIRE(lte_i32_max.contains(-1));
			STATIC_REQUIRE(lte_i32_max.contains(I32Min));
			STATIC_REQUIRE(lte_i32_max.contains(I64Min));
			STATIC_REQUIRE(lte_i32_max.contains(F32Min));
			STATIC_REQUIRE(lte_i32_max.contains(F64Min));

			constexpr UB lt_i32_max{I32Max, false};
			STATIC_REQUIRE(!lt_i32_max.contains(F64Max));
			STATIC_REQUIRE(!lt_i32_max.contains(F32Max));
			STATIC_REQUIRE(!lt_i32_max.contains(U64Max));
			STATIC_REQUIRE(!lt_i32_max.contains(I64Max));
			STATIC_REQUIRE(!lt_i32_max.contains(U32Max));
			STATIC_REQUIRE(!lt_i32_max.contains(I32Max));
			STATIC_REQUIRE(lt_i32_max.contains(1));
			STATIC_REQUIRE(lt_i32_max.contains(0));
			STATIC_REQUIRE(lt_i32_max.contains(-1));
			STATIC_REQUIRE(lt_i32_max.contains(I32Min));
			STATIC_REQUIRE(lt_i32_max.contains(I64Min));
			STATIC_REQUIRE(lt_i32_max.contains(F32Min));
			STATIC_REQUIRE(lt_i32_max.contains(F64Min));
		}
	}

	SECTION("by f32") {
		using LB = cray::detail::LowerBound<float>;
		using UB = cray::detail::UpperBound<float>;

		SECTION("min") {
			constexpr LB gt_f32_min{F32Min, false};
			STATIC_REQUIRE(gt_f32_min.contains(F64Max));
			STATIC_REQUIRE(gt_f32_min.contains(F32Max));
			STATIC_REQUIRE(gt_f32_min.contains(U64Max));
			STATIC_REQUIRE(gt_f32_min.contains(I64Max));
			STATIC_REQUIRE(gt_f32_min.contains(U32Max));
			STATIC_REQUIRE(gt_f32_min.contains(I32Max));
			STATIC_REQUIRE(gt_f32_min.contains(1));
			STATIC_REQUIRE(gt_f32_min.contains(0));
			STATIC_REQUIRE(gt_f32_min.contains(-1));
			STATIC_REQUIRE(gt_f32_min.contains(I32Min));
			STATIC_REQUIRE(gt_f32_min.contains(I64Min));
			STATIC_REQUIRE(!gt_f32_min.contains(F32Min));
			STATIC_REQUIRE(!gt_f32_min.contains(F64Min));

			constexpr LB gte_f32_min{F32Min, true};
			STATIC_REQUIRE(gte_f32_min.contains(F64Max));
			STATIC_REQUIRE(gte_f32_min.contains(F32Max));
			STATIC_REQUIRE(gte_f32_min.contains(U64Max));
			STATIC_REQUIRE(gte_f32_min.contains(I64Max));
			STATIC_REQUIRE(gte_f32_min.contains(U32Max));
			STATIC_REQUIRE(gte_f32_min.contains(I32Max));
			STATIC_REQUIRE(gte_f32_min.contains(1));
			STATIC_REQUIRE(gte_f32_min.contains(0));
			STATIC_REQUIRE(gte_f32_min.contains(-1));
			STATIC_REQUIRE(gte_f32_min.contains(I32Min));
			STATIC_REQUIRE(gte_f32_min.contains(I64Min));
			STATIC_REQUIRE(gte_f32_min.contains(F32Min));
			STATIC_REQUIRE(!gte_f32_min.contains(F64Min));

			constexpr UB lte_f32_min{F32Min, true};
			STATIC_REQUIRE(!lte_f32_min.contains(F64Max));
			STATIC_REQUIRE(!lte_f32_min.contains(F32Max));
			STATIC_REQUIRE(!lte_f32_min.contains(U64Max));
			STATIC_REQUIRE(!lte_f32_min.contains(I64Max));
			STATIC_REQUIRE(!lte_f32_min.contains(U32Max));
			STATIC_REQUIRE(!lte_f32_min.contains(I32Max));
			STATIC_REQUIRE(!lte_f32_min.contains(1));
			STATIC_REQUIRE(!lte_f32_min.contains(0));
			STATIC_REQUIRE(!lte_f32_min.contains(-1));
			STATIC_REQUIRE(!lte_f32_min.contains(I32Min));
			STATIC_REQUIRE(!lte_f32_min.contains(I64Min));
			STATIC_REQUIRE(lte_f32_min.contains(F32Min));
			STATIC_REQUIRE(lte_f32_min.contains(F64Min));

			constexpr UB lt_f32_min{F32Min, false};
			STATIC_REQUIRE(!lt_f32_min.contains(F64Max));
			STATIC_REQUIRE(!lt_f32_min.contains(F32Max));
			STATIC_REQUIRE(!lt_f32_min.contains(U64Max));
			STATIC_REQUIRE(!lt_f32_min.contains(I64Max));
			STATIC_REQUIRE(!lt_f32_min.contains(U32Max));
			STATIC_REQUIRE(!lt_f32_min.contains(I32Max));
			STATIC_REQUIRE(!lt_f32_min.contains(1));
			STATIC_REQUIRE(!lt_f32_min.contains(0));
			STATIC_REQUIRE(!lt_f32_min.contains(-1));
			STATIC_REQUIRE(!lt_f32_min.contains(I32Min));
			STATIC_REQUIRE(!lt_f32_min.contains(I64Min));
			STATIC_REQUIRE(!lt_f32_min.contains(F32Min));
			STATIC_REQUIRE(lt_f32_min.contains(F64Min));
		}

		SECTION("max") {
			constexpr LB gt_f32_max{F32Max, false};
			STATIC_REQUIRE(gt_f32_max.contains(F64Max));
			STATIC_REQUIRE(!gt_f32_max.contains(F32Max));
			STATIC_REQUIRE(!gt_f32_max.contains(U64Max));
			STATIC_REQUIRE(!gt_f32_max.contains(I64Max));
			STATIC_REQUIRE(!gt_f32_max.contains(U32Max));
			STATIC_REQUIRE(!gt_f32_max.contains(I32Max));
			STATIC_REQUIRE(!gt_f32_max.contains(1));
			STATIC_REQUIRE(!gt_f32_max.contains(0));
			STATIC_REQUIRE(!gt_f32_max.contains(-1));
			STATIC_REQUIRE(!gt_f32_max.contains(I32Min));
			STATIC_REQUIRE(!gt_f32_max.contains(I64Min));
			STATIC_REQUIRE(!gt_f32_max.contains(F32Min));
			STATIC_REQUIRE(!gt_f32_max.contains(F64Min));

			constexpr LB gte_f32_max{F32Max, true};
			STATIC_REQUIRE(gte_f32_max.contains(F64Max));
			STATIC_REQUIRE(gte_f32_max.contains(F32Max));
			STATIC_REQUIRE(!gte_f32_max.contains(U64Max));
			STATIC_REQUIRE(!gte_f32_max.contains(I64Max));
			STATIC_REQUIRE(!gte_f32_max.contains(U32Max));
			STATIC_REQUIRE(!gte_f32_max.contains(I32Max));
			STATIC_REQUIRE(!gte_f32_max.contains(1));
			STATIC_REQUIRE(!gte_f32_max.contains(0));
			STATIC_REQUIRE(!gte_f32_max.contains(-1));
			STATIC_REQUIRE(!gte_f32_max.contains(I32Min));
			STATIC_REQUIRE(!gte_f32_max.contains(I64Min));
			STATIC_REQUIRE(!gte_f32_max.contains(F32Min));
			STATIC_REQUIRE(!gte_f32_max.contains(F64Min));

			constexpr UB lte_f32_max{F32Max, true};
			STATIC_REQUIRE(!lte_f32_max.contains(F64Max));
			STATIC_REQUIRE(lte_f32_max.contains(F32Max));
			STATIC_REQUIRE(lte_f32_max.contains(U64Max));
			STATIC_REQUIRE(lte_f32_max.contains(I64Max));
			STATIC_REQUIRE(lte_f32_max.contains(U32Max));
			STATIC_REQUIRE(lte_f32_max.contains(I32Max));
			STATIC_REQUIRE(lte_f32_max.contains(1));
			STATIC_REQUIRE(lte_f32_max.contains(0));
			STATIC_REQUIRE(lte_f32_max.contains(-1));
			STATIC_REQUIRE(lte_f32_max.contains(I32Min));
			STATIC_REQUIRE(lte_f32_max.contains(I64Min));
			STATIC_REQUIRE(lte_f32_max.contains(F32Min));
			STATIC_REQUIRE(lte_f32_max.contains(F64Min));

			constexpr UB lt_f32_max{F32Max, false};
			STATIC_REQUIRE(!lt_f32_max.contains(F64Max));
			STATIC_REQUIRE(!lt_f32_max.contains(F32Max));
			STATIC_REQUIRE(lt_f32_max.contains(U64Max));
			STATIC_REQUIRE(lt_f32_max.contains(I64Max));
			STATIC_REQUIRE(lt_f32_max.contains(U32Max));
			STATIC_REQUIRE(lt_f32_max.contains(I32Max));
			STATIC_REQUIRE(lt_f32_max.contains(1));
			STATIC_REQUIRE(lt_f32_max.contains(0));
			STATIC_REQUIRE(lt_f32_max.contains(-1));
			STATIC_REQUIRE(lt_f32_max.contains(I32Min));
			STATIC_REQUIRE(lt_f32_max.contains(I64Min));
			STATIC_REQUIRE(lt_f32_max.contains(F32Min));
			STATIC_REQUIRE(lt_f32_max.contains(F64Min));
		}
	}
}

TEMPLATE_LIST_TEST_CASE("Bound::clamp-onZero", "", NumberTypes) {
	using LB = cray::detail::LowerBound<TestType>;
	using UB = cray::detail::UpperBound<TestType>;

	using V        = std::conditional_t<std::is_integral_v<TestType>, std::intmax_t, std::common_type_t<TestType, float>>;
	const V next_v = std::is_integral_v<TestType> ? +1 : std::nextafter(V(0), V(+1));
	const V prev_v = std::is_integral_v<TestType> ? -1 : std::nextafter(V(0), V(-1));

	using P         = std::conditional_t<std::is_integral_v<TestType>, std::intmax_t, std::common_type_t<TestType, double>>;
	const P next_vd = std::is_integral_v<TestType> ? +1 : std::nextafter(P(0), P(+1));
	const P prev_vd = std::is_integral_v<TestType> ? -1 : std::nextafter(P(0), P(-1));

	constexpr LB gte0{0, true};
	STATIC_REQUIRE(F64Max == gte0.clamp(F64Max));
	STATIC_REQUIRE(F32Max == gte0.clamp(F32Max));
	STATIC_REQUIRE(U64Max == gte0.clamp(U64Max));
	STATIC_REQUIRE(I64Max == gte0.clamp(I64Max));
	STATIC_REQUIRE(U32Max == gte0.clamp(U32Max));
	STATIC_REQUIRE(I32Max == gte0.clamp(I32Max));
	STATIC_REQUIRE(1 == gte0.clamp(1));
	REQUIRE(next0f == gte0.clamp(next0f));
	REQUIRE(next0d == gte0.clamp(next0d));
	STATIC_REQUIRE(0 == gte0.clamp(0));
	REQUIRE(0 == gte0.clamp(prev0d));
	REQUIRE(0 == gte0.clamp(prev0f));
	STATIC_REQUIRE(0 == gte0.clamp(-1));
	STATIC_REQUIRE(0 == gte0.clamp(I32Min));
	STATIC_REQUIRE(0 == gte0.clamp(I64Min));
	STATIC_REQUIRE(0 == gte0.clamp(F32Min));
	STATIC_REQUIRE(0 == gte0.clamp(F64Min));

	constexpr LB gt0{0, false};
	REQUIRE(F64Max == gt0.clamp(F64Max));
	REQUIRE(F32Max == gt0.clamp(F32Max));
	REQUIRE(U64Max == gt0.clamp(U64Max));
	REQUIRE(I64Max == gt0.clamp(I64Max));
	REQUIRE(U32Max == gt0.clamp(U32Max));
	REQUIRE(I32Max == gt0.clamp(I32Max));
	REQUIRE(1 == gt0.clamp(1));
	REQUIRE(next0f == gt0.clamp(next0f));
	REQUIRE(next0d == gt0.clamp(next0d));
	REQUIRE(next_v == gt0.clamp(0));
	REQUIRE(next_vd == gt0.clamp(prev0d));
	REQUIRE(next_v == gt0.clamp(prev0f));
	REQUIRE(next_v == gt0.clamp(-1));
	REQUIRE(next_v == gt0.clamp(I32Min));
	REQUIRE(next_v == gt0.clamp(I64Min));
	REQUIRE(next_v == gt0.clamp(F32Min));
	REQUIRE(next_vd == gt0.clamp(F64Min));

	// It is ill-formed if common type is unsigned.
	constexpr UB lt0{0, false};
	if constexpr(std::is_unsigned_v<TestType>) {
		STATIC_REQUIRE(-1. == lt0.clamp(F64Max));
		STATIC_REQUIRE(-1.f == lt0.clamp(F32Max));
		STATIC_REQUIRE_FALSE((0 > lt0.clamp(U64Max)) && (-1 == lt0.clamp(U64Max)));
		STATIC_REQUIRE(-1 == lt0.clamp(I64Max));
		STATIC_REQUIRE_FALSE((0 > lt0.clamp(U32Max)) && (-1 == lt0.clamp(U32Max)));
		STATIC_REQUIRE_FALSE((0 > lt0.clamp(I32Max)) && (-1 == lt0.clamp(I32Max)));
		STATIC_REQUIRE_FALSE((0 > lt0.clamp(1)) && (-1 == lt0.clamp(1)));
		REQUIRE(-1.f == lt0.clamp(next0f));
		REQUIRE(-1. == lt0.clamp(next0d));
		STATIC_REQUIRE_FALSE((0 > lt0.clamp(0)) && (-1 == lt0.clamp(0)));
		REQUIRE(prev0d == lt0.clamp(prev0d));
		REQUIRE(prev0f == lt0.clamp(prev0f));
		STATIC_REQUIRE_FALSE((0 > lt0.clamp(-1)) && (-1 == lt0.clamp(-1)));
		STATIC_REQUIRE(I32Min == lt0.clamp(I32Min));
		STATIC_REQUIRE(I64Min == lt0.clamp(I64Min));
		STATIC_REQUIRE(F32Min == lt0.clamp(F32Min));
		STATIC_REQUIRE(F64Min == lt0.clamp(F64Min));
	} else if constexpr(std::is_integral_v<TestType>) {
		STATIC_REQUIRE(-1. == lt0.clamp(F64Max));
		STATIC_REQUIRE(-1.f == lt0.clamp(F32Max));
		if constexpr(sizeof(TestType) <= 8) {
			STATIC_REQUIRE_FALSE((0 > lt0.clamp(U64Max)) && (-1 == lt0.clamp(U64Max)));
		}
		STATIC_REQUIRE(-1 == lt0.clamp(I64Max));
		if constexpr(sizeof(TestType) <= 4) {
			STATIC_REQUIRE_FALSE((0 > lt0.clamp(U32Max)) && (-1 == lt0.clamp(U32Max)));
		}
		STATIC_REQUIRE(-1 == lt0.clamp(I32Max));
		STATIC_REQUIRE(-1 == lt0.clamp(1));
		REQUIRE(-1.f == lt0.clamp(next0f));
		REQUIRE(-1. == lt0.clamp(next0d));
		STATIC_REQUIRE(-1 == lt0.clamp(0));
		REQUIRE(prev0d == lt0.clamp(prev0d));
		REQUIRE(prev0f == lt0.clamp(prev0f));
		STATIC_REQUIRE(-1 == lt0.clamp(-1));
		STATIC_REQUIRE(I32Min == lt0.clamp(I32Min));
		STATIC_REQUIRE(I64Min == lt0.clamp(I64Min));
		STATIC_REQUIRE(F32Min == lt0.clamp(F32Min));
		STATIC_REQUIRE(F64Min == lt0.clamp(F64Min));
	} else {
		REQUIRE(prev_vd == lt0.clamp(F64Max));
		REQUIRE(prev_v == lt0.clamp(F32Max));
		REQUIRE(prev_v == lt0.clamp(U64Max));
		REQUIRE(prev_v == lt0.clamp(I64Max));
		REQUIRE(prev_v == lt0.clamp(U32Max));
		REQUIRE(prev_v == lt0.clamp(I32Max));
		REQUIRE(prev_v == lt0.clamp(1));
		REQUIRE(prev_v == lt0.clamp(next0f));
		REQUIRE(prev_vd == lt0.clamp(next0d));
		REQUIRE(prev_v == lt0.clamp(0));
		REQUIRE(prev0d == lt0.clamp(prev0d));
		REQUIRE(prev0f == lt0.clamp(prev0f));
		STATIC_REQUIRE(-1 == lt0.clamp(-1));
		STATIC_REQUIRE(I32Min == lt0.clamp(I32Min));
		STATIC_REQUIRE(I64Min == lt0.clamp(I64Min));
		STATIC_REQUIRE(F32Min == lt0.clamp(F32Min));
		STATIC_REQUIRE(F64Min == lt0.clamp(F64Min));
	}

	constexpr UB lte0{0, true};
	STATIC_REQUIRE(0 == lte0.clamp(F64Max));
	STATIC_REQUIRE(0 == lte0.clamp(F32Max));
	STATIC_REQUIRE(0 == lte0.clamp(U64Max));
	STATIC_REQUIRE(0 == lte0.clamp(I64Max));
	STATIC_REQUIRE(0 == lte0.clamp(U32Max));
	STATIC_REQUIRE(0 == lte0.clamp(I32Max));
	STATIC_REQUIRE(0 == lte0.clamp(1));
	REQUIRE(0 == lte0.clamp(next0f));
	REQUIRE(0 == lte0.clamp(next0d));
	STATIC_REQUIRE(0 == lte0.clamp(0));
	REQUIRE(prev0d == lte0.clamp(prev0d));
	REQUIRE(prev0f == lte0.clamp(prev0f));
	STATIC_REQUIRE(-1 == lte0.clamp(-1));
	STATIC_REQUIRE(I32Min == lte0.clamp(I32Min));
	STATIC_REQUIRE(I64Min == lte0.clamp(I64Min));
	STATIC_REQUIRE(F32Min == lte0.clamp(F32Min));
	STATIC_REQUIRE(F64Min == lte0.clamp(F64Min));
}

TEMPLATE_LIST_TEST_CASE("Bound::operator-comparision", "", NumberTypePairs) {
	using LB = cray::detail::LowerBound<typename TestType::first_type>;
	using UB = cray::detail::UpperBound<typename TestType::second_type>;

	STATIC_REQUIRE(LB{0, true} == LB{0, true});
	STATIC_REQUIRE(LB{0, true} != LB{0, false});
	STATIC_REQUIRE(LB{0, false} != LB{0, true});
	STATIC_REQUIRE(LB{0, false} == LB{0, false});

	STATIC_REQUIRE(UB{0, true} == UB{0, true});
	STATIC_REQUIRE(UB{0, true} != UB{0, false});
	STATIC_REQUIRE(UB{0, false} != UB{0, true});
	STATIC_REQUIRE(UB{0, false} == UB{0, false});

	// - [0]-->
	//    [1]-->
	STATIC_REQUIRE(LB{0, true} < LB{1, true});
	STATIC_REQUIRE(LB{1, true} > LB{0, true});

	// - [0]-->
	//   (0)-->
	STATIC_REQUIRE(LB{0, true} < LB{1, false});
	STATIC_REQUIRE(LB{1, false} > LB{0, true});

	// - (0)-->
	//    (1)-->
	STATIC_REQUIRE(LB{0, false} < LB{1, false});
	STATIC_REQUIRE(LB{1, false} > LB{0, false});

	//   <--[0]
	// +  <--[1]
	STATIC_REQUIRE(UB{0, true} < UB{1, true});
	STATIC_REQUIRE(UB{1, true} > UB{0, true});

	// + <--[0]
	//   <--(0)
	STATIC_REQUIRE(UB{0, false} < UB{0, true});
	STATIC_REQUIRE(UB{0, true} > UB{0, false});

	//   <--(0)
	// +  <--(1)
	STATIC_REQUIRE(UB{0, false} < UB{1, false});
	STATIC_REQUIRE(UB{1, false} > UB{0, false});
}

TEMPLATE_LIST_TEST_CASE("Interval::empty", "", NumberTypes) {
	constexpr auto empty = cray::detail::Interval<TestType>::Empty();
	STATIC_REQUIRE(empty.empty());
	STATIC_REQUIRE(!empty.contains(I64Max));
	STATIC_REQUIRE(!empty.contains(I32Max));
	STATIC_REQUIRE(!empty.contains(1));
	STATIC_REQUIRE(!empty.contains(0));
	STATIC_REQUIRE(!empty.contains(-1));
	STATIC_REQUIRE(!empty.contains(I32Min));
	STATIC_REQUIRE(!empty.contains(I64Min));
}

TEST_CASE("Interval::contains") {
	constexpr auto i32 = cray::detail::Interval<std::int32_t>::of<std::int32_t>();
	STATIC_REQUIRE(!i32.contains(F64Max));
	STATIC_REQUIRE(!i32.contains(F32Max));
	STATIC_REQUIRE(!i32.contains(U64Max));
	STATIC_REQUIRE(!i32.contains(I64Max));
	STATIC_REQUIRE(!i32.contains(U32Max));
	STATIC_REQUIRE(i32.contains(I32Max));
	STATIC_REQUIRE(i32.contains(1));
	STATIC_REQUIRE(i32.contains(0));
	STATIC_REQUIRE(i32.contains(-1));
	STATIC_REQUIRE(i32.contains(I32Min));
	STATIC_REQUIRE(!i32.contains(I64Min));
	STATIC_REQUIRE(!i32.contains(F32Min));
	STATIC_REQUIRE(!i32.contains(F64Min));

	// Equivalent to Interval<std::uint32_t>.
	constexpr auto u32 = cray::detail::Interval<std::int32_t>::of<std::uint32_t>();
	STATIC_REQUIRE(!i32.contains(F64Max));
	STATIC_REQUIRE(!i32.contains(F32Max));
	STATIC_REQUIRE(!u32.contains(U64Max));
	STATIC_REQUIRE(!u32.contains(I64Max));
	STATIC_REQUIRE(u32.contains(U32Max));
	STATIC_REQUIRE(u32.contains(I32Max));
	STATIC_REQUIRE(u32.contains(1));
	STATIC_REQUIRE(u32.contains(0));
	STATIC_REQUIRE(!u32.contains(-1));
	STATIC_REQUIRE(!u32.contains(I32Min));
	STATIC_REQUIRE(!u32.contains(I64Min));
	STATIC_REQUIRE(!u32.contains(F32Min));
	STATIC_REQUIRE(!u32.contains(F64Min));

	constexpr auto f32 = cray::detail::Interval<float>::of<float>();
	STATIC_REQUIRE(!f32.contains(F64Max));
	STATIC_REQUIRE(f32.contains(F32Max));
	STATIC_REQUIRE(f32.contains(U64Max));
	STATIC_REQUIRE(f32.contains(I64Max));
	STATIC_REQUIRE(f32.contains(U32Max));
	STATIC_REQUIRE(f32.contains(I32Max));
	STATIC_REQUIRE(f32.contains(1));
	STATIC_REQUIRE(f32.contains(0));
	STATIC_REQUIRE(f32.contains(-1));
	STATIC_REQUIRE(f32.contains(I32Min));
	STATIC_REQUIRE(f32.contains(I64Min));
	STATIC_REQUIRE(f32.contains(F32Min));
	STATIC_REQUIRE(!f32.contains(F64Min));
}

TEST_CASE("Interval::clamp") {
	using cray::detail::Interval;

	constexpr Interval<std::int32_t> all;
	REQUIRE(F64Max == all.clamp(F64Max));
	REQUIRE(F32Max == all.clamp(F32Max));
	REQUIRE(U64Max == all.clamp(U64Max));
	REQUIRE(I64Max == all.clamp(I64Max));
	REQUIRE(U32Max == all.clamp(U32Max));
	REQUIRE(I32Max == all.clamp(I32Max));
	REQUIRE(1 == all.clamp(1));
	REQUIRE(0 == all.clamp(0));
	REQUIRE(-1 == all.clamp(-1));
	REQUIRE(I32Min == all.clamp(I32Min));
	REQUIRE(I64Min == all.clamp(I64Min));
	REQUIRE(F32Min == all.clamp(F32Min));
	REQUIRE(F64Min == all.clamp(F64Min));

	constexpr auto i32 = Interval<std::int32_t>::of<std::int32_t>();
	REQUIRE(I32Max == i32.clamp(F64Max));
	REQUIRE(I32Max == i32.clamp(F32Max));
	REQUIRE(I32Max == i32.clamp(U64Max));
	REQUIRE(I32Max == i32.clamp(I64Max));
	REQUIRE(I32Max == i32.clamp(U32Max));
	REQUIRE(I32Max == i32.clamp(I32Max));
	REQUIRE(1 == i32.clamp(1));
	REQUIRE(0 == i32.clamp(0));
	REQUIRE(-1 == i32.clamp(-1));
	REQUIRE(I32Min == i32.clamp(I32Min));
	REQUIRE(I32Min == i32.clamp(I64Min));
	REQUIRE(I32Min == i32.clamp(F32Min));
	REQUIRE(I32Min == i32.clamp(F64Min));

	constexpr auto f32 = Interval<float>::of<float>();
	REQUIRE(F32Max == f32.clamp(F32Max));
	REQUIRE(F32Max == f32.clamp(F64Max));
	REQUIRE(U64Max == f32.clamp(U64Max));
	REQUIRE(I64Max == f32.clamp(I64Max));
	REQUIRE(U32Max == f32.clamp(U32Max));
	REQUIRE(I32Max == f32.clamp(I32Max));
	REQUIRE(1 == f32.clamp(1));
	REQUIRE(0 == f32.clamp(0));
	REQUIRE(-1 == f32.clamp(-1));
	REQUIRE(I32Min == f32.clamp(I32Min));
	REQUIRE(I64Min == f32.clamp(I64Min));
	REQUIRE(F32Min == f32.clamp(F32Min));
	REQUIRE(F32Min == f32.clamp(F64Min));
}

TEST_CASE("Interval::intersect") {
	using Interval = cray::detail::Interval<std::int32_t>;
	using LB       = Interval::LowerBoundType;
	using UB       = Interval::UpperBoundType;

	auto const expect = [](Interval const lhs, Interval const rhs, Interval const rst) constexpr -> bool {
		return (lhs.intersect(rhs) == rst) && (rhs.intersect(lhs) == rst);
	};

	auto const expect_empty = [](Interval const lhs, Interval const rhs) constexpr -> bool {
		return lhs.intersect(rhs).empty() && rhs.intersect(lhs).empty();
	};

	STATIC_REQUIRE(expect(                     // -1   0  +1  +2
	    Interval{LB{-1, true}, std::nullopt},  //  [<-----------
	    Interval{LB{-1, true}, std::nullopt},  //  [<-----------
	    Interval{LB{-1, true}, std::nullopt}   //  [<-----------
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, std::nullopt},   //  [<-----------
	    Interval{LB{-1, false}, std::nullopt},  //  (<-----------
	    Interval{LB{-1, false}, std::nullopt}   //  (<-----------
	    ));

	STATIC_REQUIRE(expect(                     // -1   0  +1  +2
	    Interval{LB{+2, true}, std::nullopt},  //  ----------->]
	    Interval{LB{+2, true}, std::nullopt},  //  ----------->]
	    Interval{LB{+2, true}, std::nullopt}   //  ----------->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{+2, true}, std::nullopt},   //  ----------->]
	    Interval{LB{+2, false}, std::nullopt},  //  ----------->)
	    Interval{LB{+2, false}, std::nullopt}   //  ----------->)
	    ));

	STATIC_REQUIRE(expect(                     // -1   0  +1  +2
	    Interval{LB{-1, true}, std::nullopt},  //  [<-----------
	    Interval{LB{+0, true}, std::nullopt},  //      [<-------
	    Interval{LB{+0, true}, std::nullopt}   //      [<-------
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, false}, std::nullopt},  //  (<-----------
	    Interval{LB{+0, true}, std::nullopt},   //      [<-------
	    Interval{LB{+0, true}, std::nullopt}    //      [<-------
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, std::nullopt},   //  [<-----------
	    Interval{LB{+0, false}, std::nullopt},  //      (<-------
	    Interval{LB{+0, false}, std::nullopt}   //      (<-------
	    ));

	STATIC_REQUIRE(expect(                     // -1   0  +1  +2
	    Interval{std::nullopt, UB{+2, true}},  //  ----------->]
	    Interval{std::nullopt, UB{+1, true}},  //  ------->]
	    Interval{std::nullopt, UB{+1, true}}   //  ------->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{std::nullopt, UB{+2, false}},  //  ----------->)
	    Interval{std::nullopt, UB{+1, true}},   //  ------->]
	    Interval{std::nullopt, UB{+1, true}}    //  ------->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{std::nullopt, UB{+2, true}},   //  ----------->]
	    Interval{std::nullopt, UB{+1, false}},  //  ------->)
	    Interval{std::nullopt, UB{+1, false}}   //  ------->)
	    ));

	STATIC_REQUIRE(expect(                     // -1   0  +1  +2
	    Interval{LB{-1, true}, std::nullopt},  //  [<-----------
	    Interval{std::nullopt, UB{+2, true}},  //  ----------->]
	    Interval{LB{-1, true}, UB{+2, true}}   //  [<--------->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, false}, std::nullopt},  //  (<-----------
	    Interval{std::nullopt, UB{+2, true}},   //  ----------->]
	    Interval{LB{-1, false}, UB{+2, true}}   //  (<--------->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, std::nullopt},   //  [<-----------
	    Interval{std::nullopt, UB{+2, false}},  //  ----------->)
	    Interval{LB{-1, true}, UB{+2, false}}   //  [<--------->)
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, false}, std::nullopt},  //  (<-----------
	    Interval{std::nullopt, UB{+2, false}},  //  ----------->)
	    Interval{LB{-1, false}, UB{+2, false}}  //  (<--------->)
	    ));

	STATIC_REQUIRE(expect(                     // -1   0  +1  +2
	    Interval{LB{-1, true}, std::nullopt},  //  [<-----------
	    Interval{LB{-1, true}, UB{+2, true}},  //  [<--------->]
	    Interval{LB{-1, true}, UB{+2, true}}   //  [<--------->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, std::nullopt},   //  [<-----------
	    Interval{LB{-1, true}, UB{+2, false}},  //  [<--------->)
	    Interval{LB{-1, true}, UB{+2, false}}   //  [<--------->)
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, std::nullopt},   //  [<-----------
	    Interval{LB{-1, false}, UB{+2, true}},  //  (<--------->]
	    Interval{LB{-1, false}, UB{+2, true}}   //  (<--------->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, false}, std::nullopt},  //  (<-----------
	    Interval{LB{-1, true}, UB{+2, true}},   //  [<--------->]
	    Interval{LB{-1, false}, UB{+2, true}}   //  (<--------->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, false}, std::nullopt},  //  (<-----------
	    Interval{LB{-1, true}, UB{+2, false}},  //  [<--------->)
	    Interval{LB{-1, false}, UB{+2, false}}  //  (<--------->)
	    ));

	STATIC_REQUIRE(expect(                     // -1   0  +1  +2
	    Interval{std::nullopt, UB{+2, true}},  //  ----------->]
	    Interval{LB{-1, true}, UB{+2, true}},  //  [<--------->]
	    Interval{LB{-1, true}, UB{+2, true}}   //  [<--------->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{std::nullopt, UB{+2, true}},   //  ----------->]
	    Interval{LB{-1, true}, UB{+2, false}},  //  [<--------->)
	    Interval{LB{-1, true}, UB{+2, false}}   //  [<--------->)
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{std::nullopt, UB{+2, true}},   //  ----------->]
	    Interval{LB{-1, false}, UB{+2, true}},  //  (<--------->]
	    Interval{LB{-1, false}, UB{+2, true}}   //  (<--------->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{std::nullopt, UB{+2, false}},  //  ----------->)
	    Interval{LB{-1, true}, UB{+2, true}},   //  [<--------->]
	    Interval{LB{-1, true}, UB{+2, false}}   //  [<--------->)
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{std::nullopt, UB{+2, false}},  //  ----------->)
	    Interval{LB{-1, false}, UB{+2, true}},  //  (<--------->]
	    Interval{LB{-1, false}, UB{+2, false}}  //  (<--------->)
	    ));

	STATIC_REQUIRE(expect(                     // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, true}},  //  [<--------->]
	    Interval{LB{-1, true}, UB{+2, true}},  //  [<--------->]
	    Interval{LB{-1, true}, UB{+2, true}}   //  [<--------->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, true}},   //  [<--------->]
	    Interval{LB{-1, false}, UB{+2, true}},  //  (<--------->]
	    Interval{LB{-1, false}, UB{+2, true}}   //  (<--------->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, true}},   //  [<--------->]
	    Interval{LB{-1, true}, UB{+2, false}},  //  [<--------->)
	    Interval{LB{-1, true}, UB{+2, false}}   //  [<--------->)
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, false}},  //  [<--------->)
	    Interval{LB{-1, false}, UB{+2, true}},  //  (<--------->]
	    Interval{LB{-1, false}, UB{+2, false}}  //  (<--------->)
	    ));

	STATIC_REQUIRE(expect(                       // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, true}},    //  [<--------->]
	    Interval{LB{-1, false}, UB{+2, false}},  //  (<--------->)
	    Interval{LB{-1, false}, UB{+2, false}}   //  (<--------->)
	    ));

	STATIC_REQUIRE(expect(                     // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, true}},  //  [<--------->]
	    Interval{LB{+0, true}, UB{+2, true}},  //      [<----->]
	    Interval{LB{+0, true}, UB{+2, true}}   //      [<----->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, true}},   //  [<--------->]
	    Interval{LB{+0, true}, UB{+2, false}},  //      [<----->)
	    Interval{LB{+0, true}, UB{+2, false}}   //      [<----->)
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, false}},  //  [<--------->)
	    Interval{LB{+0, true}, UB{+2, true}},   //      [<----->]
	    Interval{LB{+0, true}, UB{+2, false}}   //      [<----->)
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, true}},   //  [<--------->]
	    Interval{LB{+0, false}, UB{+2, true}},  //      (<----->]
	    Interval{LB{+0, false}, UB{+2, true}}   //      (<----->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, false}, UB{+2, true}},  //  (<--------->]
	    Interval{LB{+0, true}, UB{+2, true}},   //      [<----->]
	    Interval{LB{+0, true}, UB{+2, true}}    //      [<----->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, false}, UB{+2, true}},  //  (<--------->]
	    Interval{LB{+0, false}, UB{+2, true}},  //      (<----->]
	    Interval{LB{+0, false}, UB{+2, true}}   //      (<----->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, false}},  //  [<--------->)
	    Interval{LB{+0, false}, UB{+2, true}},  //      (<----->]
	    Interval{LB{+0, false}, UB{+2, false}}  //      (<----->)
	    ));

	STATIC_REQUIRE(expect(                       // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, true}},    //  [<--------->]
	    Interval{LB{+0, false}, UB{+2, false}},  //      (<----->)
	    Interval{LB{+0, false}, UB{+2, false}}   //      (<----->)
	    ));

	STATIC_REQUIRE(expect(                     // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+1, true}},  //  [<----->]
	    Interval{LB{+0, true}, UB{+2, true}},  //      [<------>]
	    Interval{LB{+0, true}, UB{+1, true}}   //      [<->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+1, true}},   //  [<----->]
	    Interval{LB{+0, false}, UB{+2, true}},  //      (<------>]
	    Interval{LB{+0, false}, UB{+1, true}}   //      (<->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+1, false}},  //  [<----->)
	    Interval{LB{+0, true}, UB{+2, true}},   //      [<------>]
	    Interval{LB{+0, true}, UB{+1, false}}   //      [<->)
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+1, false}},  //  [<----->)
	    Interval{LB{+0, false}, UB{+2, true}},  //      (<------>]
	    Interval{LB{+0, false}, UB{+1, false}}  //      (<->)
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, false}, UB{+1, true}},  //  (<----->]
	    Interval{LB{+0, true}, UB{+2, false}},  //      [<------>)
	    Interval{LB{+0, true}, UB{+1, true}}    //      [<->]
	    ));

	STATIC_REQUIRE(expect(                     // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, true}},  //  [<--------->]
	    Interval{LB{+0, true}, UB{+1, true}},  //      [<->]
	    Interval{LB{+0, true}, UB{+1, true}}   //      [<->]
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, true}},   //  [<--------->]
	    Interval{LB{+0, true}, UB{+1, false}},  //      [<->)
	    Interval{LB{+0, true}, UB{+1, false}}   //      [<->)
	    ));

	STATIC_REQUIRE(expect(                      // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, true}},   //  [<--------->]
	    Interval{LB{+0, false}, UB{+1, true}},  //      (<->]
	    Interval{LB{+0, false}, UB{+1, true}}   //      (<->]
	    ));

	STATIC_REQUIRE(expect(                       // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+2, true}},    //  [<--------->]
	    Interval{LB{+0, false}, UB{+1, false}},  //      (<->)
	    Interval{LB{+0, false}, UB{+1, false}}   //      (<->)
	    ));

	STATIC_REQUIRE(expect(                       // -1   0  +1  +2
	    Interval{LB{-1, false}, UB{+2, false}},  //  (<--------->)
	    Interval{LB{+0, true}, UB{+1, true}},    //      [<->]
	    Interval{LB{+0, true}, UB{+1, true}}     //      [<->]
	    ));

	STATIC_REQUIRE(expect(                       // -1   0  +1  +2
	    Interval{LB{-1, false}, UB{+2, false}},  //  (<--------->)
	    Interval{LB{+0, true}, UB{+1, true}},    //      [<->]
	    Interval{LB{+0, true}, UB{+1, true}}     //      [<->]
	    ));

	STATIC_REQUIRE(expect(                     // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+0, true}},  //  [<->]
	    Interval{LB{+0, true}, UB{+1, true}},  //      [<->]
	    Interval{LB{+0, true}, UB{+0, true}}   //      +
	    ));

	STATIC_REQUIRE(expect_empty(               // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+0, true}},  //  [<->]
	    Interval{LB{+0, false}, UB{+1, true}}  //      (<->]
	    ));

	STATIC_REQUIRE(expect_empty(                // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+0, false}},  //  [<->)
	    Interval{LB{+0, true}, UB{+1, true}}    //      [<->]
	    ));

	STATIC_REQUIRE(expect_empty(                // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+0, false}},  //  [<->)
	    Interval{LB{+0, false}, UB{+1, true}}   //      (<->]
	    ));

	STATIC_REQUIRE(expect_empty(               // -1   0  +1  +2
	    Interval{LB{-1, true}, UB{+0, true}},  //  [<->]
	    Interval{LB{+1, true}, UB{+2, true}}   //          [<->]
	    ));
}

TEST_CASE("IntervalPlaceHolder") {
	using cray::detail::LowerBound;
	using cray::detail::UpperBound;
	using cray::detail::Interval;
	using cray::x;

	auto const eq = [](auto const& lhs, auto const& rhs) constexpr -> bool {
		return lhs == rhs;
	};

	REQUIRE(eq(x < 1, Interval<int>{std::nullopt, UpperBound<int>{1, false}}));
	REQUIRE(eq(x <= 1, Interval<int>{std::nullopt, UpperBound<int>{1, true}}));
	REQUIRE(eq(-1 < x, Interval<int>{LowerBound<int>{-1, false}, std::nullopt}));
	REQUIRE(eq(-1 <= x, Interval<int>{LowerBound<int>{-1, true}, std::nullopt}));
	REQUIRE(eq(-1 < x < 1, Interval<int>{LowerBound<int>{-1, false}, UpperBound<int>{1, false}}));
	REQUIRE(eq(-1 < x <= 1, Interval<int>{LowerBound<int>{-1, false}, UpperBound<int>{1, true}}));
	REQUIRE(eq(-1 <= x < 1, Interval<int>{LowerBound<int>{-1, true}, UpperBound<int>{1, false}}));
	REQUIRE(eq(-1 <= x <= 1, Interval<int>{LowerBound<int>{-1, true}, UpperBound<int>{1, true}}));

	REQUIRE(eq(-1 < x < float(1), Interval<float>{LowerBound<float>{-1, false}, UpperBound<float>{1, false}}));
	REQUIRE(eq(float(-1) < x < 1, Interval<float>{LowerBound<float>{-1, false}, UpperBound<float>{1, false}}));
	REQUIRE(eq(-1 < x <= float(1), Interval<float>{LowerBound<float>{-1, false}, UpperBound<float>{1, true}}));
	REQUIRE(eq(float(-1) < x <= 1, Interval<float>{LowerBound<float>{-1, false}, UpperBound<float>{1, true}}));
	REQUIRE(eq(-1 <= x < float(1), Interval<float>{LowerBound<float>{-1, true}, UpperBound<float>{1, false}}));
	REQUIRE(eq(float(-1) <= x < 1, Interval<float>{LowerBound<float>{-1, true}, UpperBound<float>{1, false}}));
	REQUIRE(eq(-1 <= x <= float(1), Interval<float>{LowerBound<float>{-1, true}, UpperBound<float>{1, true}}));
	REQUIRE(eq(float(-1) <= x <= 1, Interval<float>{LowerBound<float>{-1, true}, UpperBound<float>{1, true}}));
}

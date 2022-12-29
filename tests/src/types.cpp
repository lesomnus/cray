#include <cstddef>
#include <optional>
#include <string>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

#include <cray/types.hpp>

struct Dummy {
};

TEST_CASE("TypeFor") {
	using namespace cray;

	STATIC_REQUIRE(Type::Nil == TypeFor<std::nullptr_t>);

	STATIC_REQUIRE(Type::Bool == TypeFor<bool>);

	STATIC_REQUIRE(Type::Int == TypeFor<char>);
	STATIC_REQUIRE(Type::Int == TypeFor<short>);
	STATIC_REQUIRE(Type::Int == TypeFor<int>);
	STATIC_REQUIRE(Type::Int == TypeFor<long>);
	STATIC_REQUIRE(Type::Int == TypeFor<long long>);
	STATIC_REQUIRE(Type::Int == TypeFor<unsigned char>);
	STATIC_REQUIRE(Type::Int == TypeFor<unsigned short>);
	STATIC_REQUIRE(Type::Int == TypeFor<unsigned int>);
	STATIC_REQUIRE(Type::Int == TypeFor<unsigned long>);
	STATIC_REQUIRE(Type::Int == TypeFor<unsigned long long>);

	STATIC_REQUIRE(Type::Num == TypeFor<float>);
	STATIC_REQUIRE(Type::Num == TypeFor<double>);

	STATIC_REQUIRE(Type::Str == TypeFor<std::string>);

	STATIC_REQUIRE(Type::Map == TypeFor<Dummy>);
}

TEST_CASE("IsScalarType") {
	using namespace cray;

	STATIC_REQUIRE(IsScalarType<Type::Nil>);
	STATIC_REQUIRE(IsScalarType<Type::Bool>);
	STATIC_REQUIRE(IsScalarType<Type::Int>);
	STATIC_REQUIRE(IsScalarType<Type::Num>);
	STATIC_REQUIRE(not IsScalarType<Type::Map>);
	STATIC_REQUIRE(not IsScalarType<Type::List>);
}

TEST_CASE("IsOptional") {
	using namespace cray::detail;

	STATIC_REQUIRE(IsOptional<std::optional<int>>);
	STATIC_REQUIRE(not IsOptional<int>);
}

TEST_CASE("Reference") {
	using namespace cray;

	STATIC_REQUIRE(Reference::IsIndex<std::size_t>);
	STATIC_REQUIRE(not Reference::IsIndex<decltype(42)>);
	STATIC_REQUIRE(not Reference::IsIndex<std::string>);

	STATIC_REQUIRE(Reference::IsKey<std::string>);
	STATIC_REQUIRE(not Reference::IsKey<decltype("key")>);
	STATIC_REQUIRE(not Reference::IsKey<decltype(42)>);

	REQUIRE(Reference(42).isIndex());
	REQUIRE(42 == Reference(42).index());

	REQUIRE(Reference("hypnos").isKey());
	REQUIRE("hypnos" == Reference("hypnos").key());

	auto const be = [](auto expected) {
		return [&](auto value) {
			using T = std::decay_t<decltype(expected)>;
			using U = std::decay_t<decltype(value)>;

			if constexpr(std::is_same_v<T, U>) {
				return expected == value;
			} else {
				return false;
			}
		};
	};
	REQUIRE(Reference(42).visit(be(std::size_t(42))));
	REQUIRE(Reference("hypnos").visit(be(std::string("hypnos"))));
}

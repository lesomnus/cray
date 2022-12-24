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

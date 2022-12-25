#include <iostream>
#include <sstream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <cray/detail/props.hpp>
#include <cray/node.hpp>
#include <cray/report.hpp>
#include <cray/source.hpp>

TEST_CASE("Annotation") {
	using namespace cray;

	Node node(Source::make(42));

	Annotation  annotation;
	std::string answer;

	SECTION("no annotations") {
		answer = R"(
42
)";
	}

	SECTION("title and description") {
		annotation = Annotation{
		    .title       = "Title",
		    .description = "Description",
		};

		answer = R"(
# Title
# | Description
42
)";
	}

	SECTION("deprecated") {
		annotation = Annotation{
		    .is_deprecated = true,
		};

		answer = R"(
# ⚠️ DEPRECATED
42
)";
	}

	SECTION("title and description with deprecated") {
		annotation = Annotation{
		    .title         = "Title",
		    .description   = "Description",
		    .is_deprecated = true,
		};

		answer = R"(
# Title
# ⚠️ DEPRECATED
# | Description
42
)";
	}

	node.is<Type::Int>(annotation);

	std::stringstream rst;
	rst << std::endl;
	report::asYaml(rst, node);
	rst << std::endl;

	REQUIRE(answer == std::move(rst).str());
}

TEST_CASE("IntProp") {
	using namespace cray;

	Node node(Source::make(42));

	auto        describer = node.is<Type::Int>();
	std::string answer;

	SECTION("no constraints") {
		answer = R"(
42
)";
	}

	SECTION("interval") {
		describer.interval(-1 < x <= 1);

		answer = R"(
# • { x ∈ Z | -1 < x ≤ 1 }
  # ⚠️ INVALID VALUE
)";
	}

	SECTION("multipleOf") {
		describer.mutipleOf(7);

		answer = R"(
# • { x ∈ Z | x / 7 ∈ Z }
42
)";
	}

	SECTION("interval and multipleOf") {
		describer.interval(x <= 42).mutipleOf(6);

		answer = R"(
# • { x ∈ Z | (x ≤ 42) ∧ (x / 6 ∈ Z) }
42
)";
	}

	std::stringstream rst;
	rst << std::endl;
	report::asYaml(rst, node);
	rst << std::endl;

	REQUIRE(answer == std::move(rst).str());
}

TEST_CASE("MonoMapProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	Node node(Source::make({_{"a", 3}}));

	auto        describer = node.is<Type::Map>().of<Type::Int>();
	std::string answer;

	SECTION("no constraints") {
		answer = R"(
a: 3
)";
	}

	SECTION("required fields") {
		describer.containing({"a", "b", "c"});
		answer = R"(
b:   # ⚠️ REQUIRED
c:   # ⚠️ REQUIRED
a: 3
)";
	}

	SECTION("annotation on next level") {
		auto describer_next = prop<Type::Int>();

		node.is<Type::Map>().of(prop<Type::Int>(Annotation{.title = "Counts"}).interval(3 < x));
		answer = R"(
# Counts
# • { x ∈ Z | 3 < x }
a:   # ⚠️ INVALID VALUE
)";
	}

	std::stringstream rst;
	rst << std::endl;
	report::asYaml(rst, node);
	// rst << std::endl; // Map prints last empty line.

	REQUIRE(answer == std::move(rst).str());
}

#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <cray/detail/props.hpp>
#include <cray/node.hpp>
#include <cray/report.hpp>
#include <cray/source.hpp>

void requireEq(std::string expected, std::string actual) {
	std::replace(expected.begin(), expected.end(), ' ', '.');
	std::replace(actual.begin(), actual.end(), ' ', '.');

	CAPTURE(expected.size());
	CAPTURE(actual.size());

	REQUIRE(expected == actual);
}

TEST_CASE("Annotation") {
	using namespace cray;

	Node node(Source::make(42));

	Annotation  annotation;
	std::string expected;

	SECTION("no annotations") {
		expected = R"(
42
)";
	}

	SECTION("title and description") {
		annotation = Annotation{
		    .title       = "Title",
		    .description = "Description",
		};

		expected = R"(
# Title
# | Description
42
)";
	}

	SECTION("deprecated") {
		annotation = Annotation{
		    .is_deprecated = true,
		};

		expected = R"(
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

		expected = R"(
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

	requireEq(expected, std::move(rst).str());
}

TEST_CASE("IntProp") {
	using namespace cray;

	Node node(Source::make(42));

	auto        describer = node.is<Type::Int>();
	std::string expected;

	SECTION("no constraints") {
		expected = R"(
42
)";
	}

	SECTION("interval") {
		describer.interval(-1 < x <= 1);

		expected = R"(
# • { x ∈ Z | -1 < x ≤ 1 }

)";
	}

	SECTION("multipleOf") {
		describer.mutipleOf(7);

		expected = R"(
# • { x ∈ Z | x / 7 ∈ Z }
42
)";
	}

	SECTION("interval and multipleOf") {
		describer.interval(x <= 42).mutipleOf(6);

		expected = R"(
# • { x ∈ Z | (x ≤ 42) ∧ (x / 6 ∈ Z) }
42
)";
	}

	std::stringstream rst;
	rst << std::endl;
	report::asYaml(rst, node);
	rst << std::endl;

	requireEq(expected, std::move(rst).str());
}

TEST_CASE("PolyMapProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	Node node(Source::make({
	    _{"a", 3},
	    _{
	        "foo",
	        {
	            _{"bar", 42},
	        }},
	}));

	std::string expected;

	SECTION("no constraints") {
		node["a"].is<Type::Int>(Annotation{.title = "Title A"});
		node["b"].is<Type::Int>(Annotation{.title = "Title B"});

		expected = R"(
# Title A
a: 3
# Title B
b: 
)";
	}

	SECTION("required fields") {
		node["a"].is<Type::Int>();
		node[req("b")].is<Type::Int>();
		node["c"].is<Type::Int>().get();

		expected = R"(
a: 3
b:   # ⚠️ REQUIRED
c:   # ⚠️ REQUIRED
)";
	}

	SECTION("nested") {
		Node foo = node["foo"];
		foo["bar"].is<Type::Int>();
		foo["baz"].is<Type::Int>();
		foo["a"]["b"].is<Type::Int>();

		expected = R"(
foo: 
  bar: 42
  baz: 
  a: 
    b: 
)";
	}

	std::stringstream rst;
	rst << std::endl;
	report::asYaml(rst, node);
	rst << std::endl;

	requireEq(expected, std::move(rst).str());
}

TEST_CASE("MonoMapProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	Node node(Source::make({_{"a", 3}}));

	auto        describer = node.is<Type::Map>().of<Type::Int>();
	std::string expected;

	SECTION("no constraints") {
		expected = R"(
a: 3
)";
	}

	SECTION("required fields") {
		describer.containing({"a", "b", "c"});
		expected = R"(
b:   # ⚠️ REQUIRED
c:   # ⚠️ REQUIRED
a: 3
)";
	}

	SECTION("required fields all empty") {
		node = Node(Source::null());
		node.is<Type::Map>().of<Type::Int>().containing({"a", "b"});

		expected = R"(
a:   # ⚠️ REQUIRED
b:   # ⚠️ REQUIRED
)";
	}

	SECTION("annotation on next level") {
		auto describer_next = prop<Type::Int>();

		node.is<Type::Map>().of(prop<Type::Int>(Annotation{.title = "Counts"}).interval(3 < x));
		expected = R"(
# Counts
# • { x ∈ Z | 3 < x }
a: 
)";
	}

	std::stringstream rst;
	rst << std::endl;
	report::asYaml(rst, node);
	rst << std::endl;

	requireEq(expected, std::move(rst).str());
}

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

TEST_CASE("NilProp") {
	using namespace cray;

	Node node(Source::make(nullptr));

	auto        describer = node.is<Type::Nil>();
	std::string expected;

	SECTION("no constraints") {
		expected = R"(
~
)";
	}

	std::stringstream rst;
	rst << std::endl;
	report::asYaml(rst, node);
	rst << std::endl;

	requireEq(expected, std::move(rst).str());
}

TEST_CASE("BoolProp") {
	using namespace cray;

	Node        node;
	std::string expected;

	SECTION("true") {
		node = Node(Source::make(true));
		node.is<Type::Bool>();

		expected = R"(
true
)";
	}

	SECTION("false") {
		node = Node(Source::make(false));
		node.is<Type::Bool>();

		expected = R"(
false
)";
	}

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

TEST_CASE("NumProp") {
	using namespace cray;

	Node node(Source::make(3.14));

	auto        describer = node.is<Type::Num>();
	std::string expected;

	SECTION("no constraints") {
		expected = R"(
3.14
)";
	}

	SECTION("interval") {
		describer.interval(-1.2 < x <= 2.718);

		expected = R"(
# • { x ∈ Q | -1.2 < x ≤ 2.718 }

)";
	}

	SECTION("multipleOf") {
		describer.mutipleOf(0.02);

		expected = R"(
# • { x ∈ Q | x / 0.02 ∈ Z }
3.14
)";
	}

	SECTION("interval and multipleOf") {
		describer.interval(x <= 3.14).mutipleOf(0.005);

		expected = R"(
# • { x ∈ Q | (x ≤ 3.14) ∧ (x / 0.005 ∈ Z) }
3.14
)";
	}

	std::stringstream rst;
	rst << std::endl;
	report::asYaml(rst, node);
	rst << std::endl;

	requireEq(expected, std::move(rst).str());
}

TEST_CASE("StrProp") {
	using namespace cray;

	Node node(Source::make("hypnos"));

	auto        describer = node.is<Type::Str>();
	std::string expected;

	SECTION("no constraints") {
		expected = R"(
hypnos
)";
	}

	SECTION("interval") {
		describer.length(3 < x <= 5);

		expected = R"(
# • { |x| ∈ N | 3 < |x| ≤ 5 }

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

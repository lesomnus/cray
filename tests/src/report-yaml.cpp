#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <cray/node.hpp>
#include <cray/props.hpp>
#include <cray/report.hpp>
#include <cray/source.hpp>

template<typename T>
struct Holder {
	T value;
};

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

	SECTION("default value") {
		node = Node(Source::null());
		node.is<Type::Int>().defaultValue(42);

		expected = R"(
# 42
)";
	}

	SECTION("interval") {
		describer.interval(-1 < x <= 1);

		expected = R"(
# • { x ∈ Z | -1 < x ≤ 1 }
  # <Integer>
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

	SECTION("default value") {
		node = Node(Source::null());
		node.is<Type::Num>().defaultValue(3.14);

		expected = R"(
# 3.14
)";
	}

	SECTION("interval") {
		describer.interval(-1.2 < x <= 2.718);

		expected = R"(
# • { x ∈ Q | -1.2 < x ≤ 2.718 }
  # <Number>
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

	SECTION("default value") {
		node = Node(Source::null());
		node.is<Type::Str>().defaultValue("hypnos");

		expected = R"(
# hypnos
)";
	}

	SECTION("interval") {
		describer.length(3 < x <= 5);

		expected = R"(
# • { |x| ∈ W | 3 < |x| ≤ 5 }
  # <String>
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
b:   # <Integer>

)";
	}

	SECTION("required fields") {
		node["a"].is<Type::Int>();
		node[req("b")].is<Type::Int>();
		node["c"].is<Type::Int>().get();

		expected = R"(
a: 3
b:   # <Integer>  ⚠️ REQUIRED
c:   # <Integer>  ⚠️ REQUIRED
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
  baz:   # <Integer>
  a: 
    b:   # <Integer>
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

	SECTION("no data") {
		node = Node(Source::null());
		node.is<Type::Map>().of<Type::Int>();

		expected = R"(
# key: <Integer>
)";
	}

	SECTION("required fields") {
		describer.containing({"a", "b", "c"});
		expected = R"(
b:   # <Integer>  ⚠️ REQUIRED
c:   # <Integer>  ⚠️ REQUIRED
a: 3
)";
	}

	SECTION("required fields all empty") {
		node = Node(Source::null());
		node.is<Type::Map>().of<Type::Int>().containing({"a", "b"});

		expected = R"(
a:   # <Integer>  ⚠️ REQUIRED
b:   # <Integer>  ⚠️ REQUIRED
# key: <Integer>
)";
	}

	SECTION("annotation on next level") {
		auto describer_next = prop<Type::Int>();

		node.is<Type::Map>().of(prop<Type::Int>(Annotation{.title = "Counts"}).interval(3 < x));
		expected = R"(
# Counts
# • { x ∈ Z | 3 < x }
a:   # <Integer>
)";
	}

	SECTION("of MonoMapProp") {
		SECTION("no data") {
			node = Node(Source::null());
			node.is<Type::Map>().of(prop<Type::Map>().of<Type::Int>());

			expected = R"(
# key: <Map of Integer>
)";
		}

		SECTION("with data") {
			node = Node(Source::make({
			    _{"A", {_{"a", 1}, _{"b", 2}, _{"c", 3}}},
			    _{"B", {_{"d", 4}, _{"e", 5}, _{"f", 6}}},
			}));
			node.is<Type::Map>().of(prop<Type::Map>().of<Type::Int>());

			expected = R"(
A: 
  a: 1
  b: 2
  c: 3
B: 
  d: 4
  e: 5
  f: 6
)";
		}

		SECTION("required fields") {
			node = Node(Source::make({
			    _{"A", {_{"a", 1}}},
			    _{"B", {_{"b", 2}}},
			}));
			node.is<Type::Map>().of(
			    prop<Type::Map>().of<Type::Int>().containing({"a", "b"}));

			expected = R"(
A: 
  b:   # <Integer>  ⚠️ REQUIRED
  a: 1
B: 
  a:   # <Integer>  ⚠️ REQUIRED
  b: 2
)";
		}
	}

	SECTION("of MonoListProp") {
		node = Node(Source::null());
		node.is<Type::Map>().of(prop<Type::List>().of<Type::Int>());

		expected = R"(
# key: <List of Integer>
)";
	}

	std::stringstream rst;
	rst << std::endl;
	report::asYaml(rst, node);
	rst << std::endl;

	requireEq(expected, std::move(rst).str());
}

TEST_CASE("StructuredMapProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	Node        node(Source::null());
	std::string expected;

	SECTION("no data") {
		using H = Holder<int>;
		node.is<Type::Map>().to<H>()
		    | field("int", &H::value);

		expected = R"(
int:   # <Integer>  ⚠️ REQUIRED
)";
	}

	SECTION("with data") {
		node = Node(Source::make({_{"str", "hypnos"}}));

		using H = Holder<std::string>;
		node.is<Type::Map>().to<H>()
		    | field("str", &H::value);

		expected = R"(
str: hypnos
)";
	}

	SECTION("default value") {
		using H = Holder<std::vector<int>>;
		node.is<Type::Map>().to<H>()
		        | field("list", &H::value)
		    || H{{2, 3, 5, 7, 11}};

		expected = R"(
list: [2, 3, 5, 7, 11]
)";
	}

	std::stringstream rst;
	rst << std::endl;
	report::asYaml(rst, node);
	rst << std::endl;

	requireEq(expected, std::move(rst).str());
}

TEST_CASE("MonoListProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	Node node(Source::make({2, 3, 5}));

	auto        describer = node.is<Type::List>().of<Type::Int>();
	std::string expected;

	SECTION("no data") {
		node = Node(Source::null());
		node.is<Type::List>().of<Type::Int>();

		expected = R"(
# - <Integer>
# - ...
)";
	}

	SECTION("no constraints") {
		expected = R"(
[2, 3, 5]
)";
	}

	SECTION("default value") {
		node = Node(Source::null());
		node.is<Type::List>().of<Type::Int>().defaultValue({2, 4, 8});

		expected = R"(
# [2, 4, 8]
)";
	}

	SECTION("size") {
		describer.size(2 < x <= 5);

		expected = R"(
# • { |x| ∈ W | 2 < |x| ≤ 5 }
[2, 3, 5]
)";
	}

	SECTION("of MonoListProp") {
		SECTION("no data") {
			node = Node(Source::null());
			node.is<Type::List>().of(prop<Type::List>().of<Type::Int>());

			expected = R"(
# - <List of Integer>
# - ...
)";
		}

		SECTION("with data") {
			node = Node(Source::make({
			    {1, 2, 3},
			    {4, 5, 6},
			    {7, 8, 9},
			}));
			node.is<Type::List>().of(prop<Type::List>().of<Type::Int>());

			expected = R"(
- [1, 2, 3]
- [4, 5, 6]
- [7, 8, 9]
)";
		}
	}

	SECTION("of MonoMapProp") {
		SECTION("no data") {
			node = Node(Source::null());
			node.is<Type::List>().of(prop<Type::Map>().of<Type::Int>());

			expected = R"(
# - <Map of Integer>
# - ...
)";
		}

		SECTION("with data") {
			node = Node(Source::make({
			    {_{"a", 1}, _{"b", 2}},
			    {_{"c", 3}, _{"d", 4}},
			}));
			node.is<Type::List>().of(prop<Type::Map>().of<Type::Int>());

			expected = R"(
- 
  a: 1
  b: 2
- 
  c: 3
  d: 4
)";
		}
	}

	std::stringstream rst;
	rst << std::endl;
	report::asYaml(rst, node);
	rst << std::endl;

	requireEq(expected, std::move(rst).str());
}

struct Ingredient {
	std::string name;
	double      quantity;
};

struct Enchilada {
	bool        flag;
	int         integer;
	double      number;
	std::string string;

	std::unordered_map<std::string, std::vector<double>> extrinsics;

	std::vector<Ingredient> ingredients;
};

TEST_CASE("Enchilada") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	auto        source = Source::null();
	std::string expected;

	Node node(source);

	SECTION("no data") {
		expected = R"(
dish: 
  flag:   # <Bool>  ⚠️ REQUIRED
  int:   # <Integer>  ⚠️ REQUIRED
  num:   # <Number>  ⚠️ REQUIRED
  str:   # <String>  ⚠️ REQUIRED
  extrinsics: 
    # key: <List of Number>
)";
	}

	node["dish"].is<Type::Map>().to<Enchilada>()
	    | field("flag", &Enchilada::flag)
	    | field("int", &Enchilada::integer)
	    | field("num", &Enchilada::number)
	    | field("str", &Enchilada::string)
	    | field("extrinsics", &Enchilada::extrinsics);
	// | (field(
	//     "ingredients",
	//     prop<Type::Map>().to<Ingredient>()
	//         | field("name", &Ingredient::name)
	//         | field("qty", &Ingredient::quantity)));

	std::stringstream rst;
	rst << std::endl;
	report::asYaml(rst, node);
	rst << std::endl;

	requireEq(expected, std::move(rst).str());
}

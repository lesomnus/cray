#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <cray.hpp>

#include "testing.hpp"

class ReportTester {
   public:
	ReportTester() = default;

	~ReportTester() {
		if(!this->is_done_) {
			this->done();
		}
	}

	void done() {
		this->is_done_ = true;

		std::stringstream rst;
		rst << std::endl;
		cray::report::asYaml(rst, this->node);
		rst << std::endl;

		testing::requireReportEq(this->expected, std::move(rst).str());
	}

	cray::Node  node = cray::Node(cray::Source::null());
	std::string expected;

   private:
	bool is_done_ = false;
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

	ReportTester t;
	t.node = Node(Source::make(42));

	Annotation annotation;

	SECTION("no annotations") {
		t.expected = R"(
---
42
)";
	}

	SECTION("title and description") {
		annotation = Annotation{
		    .title       = "Title",
		    .description = "Description",
		};

		t.expected = R"(
---
# Title
# | Description
42
)";
	}

	SECTION("multiline description") {
		annotation = Annotation{
		    .description = "aaa\nbbb\nccc",
		};

		t.expected = R"(
---
# | aaa
#   bbb
#   ccc
42
)";
	}

	SECTION("deprecated") {
		annotation = Annotation{
		    .is_deprecated = true,
		};

		t.expected = R"(
---
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

		t.expected = R"(
---
# Title
# ⚠️ DEPRECATED
# | Description
42
)";
	}

	t.node.is<Type::Int>(annotation);
	t.done();
}

TEST_CASE("NilProp") {
	using namespace cray;

	ReportTester t;
	t.node.is<Type::Nil>();

	SECTION("no constraints") {
		t.expected = R"(
---
~
)";
	}

	t.done();
}

TEST_CASE("BoolProp") {
	using namespace cray;

	ReportTester t;
	t.node.is<Type::Nil>();

	SECTION("true") {
		t.node = Node(Source::make(true));

		t.expected = R"(
---
true
)";
	}

	SECTION("false") {
		t.node = Node(Source::make(false));

		t.expected = R"(
---
false
)";
	}

	t.node.is<Type::Bool>();
	t.done();
}

TEST_CASE("IntProp") {
	using namespace cray;

	ReportTester t;
	t.node = Node(Source::make(42));

	auto describer = t.node.is<Type::Int>();

	SECTION("no constraints") {
		t.expected = R"(
---
42
)";
	}

	SECTION("default value") {
		t.node = Node(Source::null());
		t.node.is<Type::Int>().defaultValue(42);

		t.expected = R"(
---
# 42
)";
	}

	SECTION("interval") {
		describer.interval(-1 < x <= 1);

		t.expected = R"(
---
# • { x ∈ Z | -1 < x ≤ 1 }
  # <Integer>
)";
	}

	SECTION("multipleOf") {
		describer.mutipleOf(7);

		t.expected = R"(
---
# • { x ∈ Z | x / 7 ∈ Z }
42
)";
	}

	SECTION("interval and multipleOf") {
		describer.interval(x <= 42).mutipleOf(6);

		t.expected = R"(
---
# • { x ∈ Z | (x ≤ 42) ∧ (x / 6 ∈ Z) }
42
)";
	}

	t.done();
}

TEST_CASE("NumProp") {
	using namespace cray;

	ReportTester t;
	t.node = Node(Source::make(3.14));

	auto describer = t.node.is<Type::Num>();

	SECTION("no constraints") {
		t.expected = R"(
---
3.14
)";
	}

	SECTION("default value") {
		t.node = Node(Source::null());
		t.node.is<Type::Num>().defaultValue(3.14);

		t.expected = R"(
---
# 3.14
)";
	}

	SECTION("interval") {
		describer.interval(-1.2 < x <= 2.718);

		t.expected = R"(
---
# • { x ∈ Q | -1.2 < x ≤ 2.718 }
  # <Number>
)";
	}

	SECTION("multipleOf") {
		describer.mutipleOf(0.02);

		t.expected = R"(
---
# • { x ∈ Q | x / 0.02 ∈ Z }
3.14
)";
	}

	SECTION("interval and multipleOf") {
		describer.interval(x <= 3.14).mutipleOf(0.005);

		t.expected = R"(
---
# • { x ∈ Q | (x ≤ 3.14) ∧ (x / 0.005 ∈ Z) }
3.14
)";
	}

	t.done();
}

TEST_CASE("StrProp") {
	using namespace cray;

	ReportTester t;
	t.node = Node(Source::make("hypnos"));

	auto describer = t.node.is<Type::Str>();

	SECTION("no constraints") {
		t.expected = R"(
---
hypnos
)";
	}

	SECTION("default value") {
		t.node = Node(Source::null());
		t.node.is<Type::Str>().defaultValue("hypnos");

		t.expected = R"(
---
# hypnos
)";
	}

	SECTION("multiline") {
		t.node = Node(Source::make(R"(aaa
bbb

ccc


)"));
		t.node.is<Type::Str>();

		t.expected = R"(
---
|
aaa
bbb

ccc


)";
	}

	SECTION("multiline default value") {
		t.node = Node(Source::null());
		t.node.is<Type::Str>().defaultValue(R"(aaa
bbb

ccc


)");

		t.expected = R"(
---

# aaa
# bbb
# 
# ccc
# 
# 
)";
	}

	SECTION("interval") {
		describer.length(3 < x <= 5);

		t.expected = R"(
---
# • { |x| ∈ W | 3 < |x| ≤ 5 }
  # <String>
)";
	}

	t.done();
}

TEST_CASE("PolyMapProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	ReportTester t;
	t.node = Node(Source::make({
	    _{"foo",
	      {
	          _{"bar", 42},
	      }},
	    _{"a", 3},
	    _{"b",
	      {
	          _{"bar", 42},
	      }},
	}));

	SECTION("no constraints") {
		t.node["a"].is<Type::Int>(Annotation{.title = "Title A"});
		t.node["b"].is<Type::Int>(Annotation{.title = "Title B"});

		t.expected = R"(
---
# Title A
a: 3

# Title B
b:   # <Integer>

)";
	}

	SECTION("required fields") {
		t.node["a"].is<Type::Int>();
		t.node[req("b")].is<Type::Int>();
		t.node["c"].is<Type::Int>().get();

		t.expected = R"(
---
a: 3
b:   # <Integer>  ⚠️ REQUIRED
c:   # <Integer>  ⚠️ REQUIRED
)";
	}

	SECTION("nested") {
		Node foo = t.node["foo"];
		foo["bar"].is<Type::Int>();
		foo["baz"].is<Type::Int>();
		foo["a"]["b"].is<Type::Int>();

		t.expected = R"(
---
foo: 
  bar: 42
  baz:   # <Integer>
  a: 
    b:   # <Integer>
)";
	}

	t.done();
}

TEST_CASE("MonoMapProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	ReportTester t;
	t.node = Node(Source::make({_{"a", 3}}));

	auto describer = t.node.is<Type::Map>().of<Type::Int>();

	SECTION("no constraints") {
		t.expected = R"(
---
a: 3
)";
	}

	SECTION("no data") {
		t.node = Node(Source::null());
		t.node.is<Type::Map>().of<Type::Int>();

		t.expected = R"(
---
# key:   # <Integer>
# ...
)";
	}

	SECTION("required fields") {
		describer.containing({"a", "b", "c"});

		t.expected = R"(
---
b:   # <Integer>  ⚠️ REQUIRED
c:   # <Integer>  ⚠️ REQUIRED
a: 3
)";
	}

	SECTION("all required fields empty") {
		t.node = Node(Source::null());
		t.node.is<Type::Map>().of<Type::Int>().containing({"a", "b"});

		t.expected = R"(
---
a:   # <Integer>  ⚠️ REQUIRED
b:   # <Integer>  ⚠️ REQUIRED
# key:   # <Integer>
# ...
)";
	}

	SECTION("annotation on next level") {
		t.node.is<Type::Map>().of(prop<Type::Int>(Annotation{.title = "Counts"}).interval(3 < x));

		t.expected = R"(
---
# Counts
# • { x ∈ Z | 3 < x }
a:   # <Integer>
)";
	}

	SECTION("of MonoMapProp") {
		SECTION("no data") {
			t.node = Node(Source::null());
			t.node.is<Type::Map>().of(prop<Type::Map>().of<Type::Int>());

			t.expected = R"(
---
# key: 
#   # key:   # <Integer>
#   # ...
# ...
)";
		}

		SECTION("with data") {
			t.node = Node(Source::make({
			    _{"A", {_{"a", 1}, _{"b", 2}, _{"c", 3}}},
			    _{"B", {_{"d", 4}, _{"e", 5}, _{"f", 6}}},
			}));
			t.node.is<Type::Map>().of(prop<Type::Map>().of<Type::Int>());

			t.expected = R"(
---
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
			t.node = Node(Source::make({
			    _{"A", {_{"a", 1}}},
			    _{"B", {_{"b", 2}}},
			}));
			t.node.is<Type::Map>().of(
			    prop<Type::Map>().of<Type::Int>().containing({"a", "b"}));

			t.expected = R"(
---
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
		t.node = Node(Source::null());
		t.node.is<Type::Map>().of(prop<Type::List>().of<Type::Int>());

		t.expected = R"(
---
# key: 
#   # -   # <Integer>
#   # - ...
# ...
)";
	}

	t.done();
}

TEST_CASE("StructuredMapProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	ReportTester t;

	std::string expected;

	SECTION("no data") {
		using H = testing::Holder<int>;

		t.node.is<Type::Map>().to<H>()
		    | field("int", &H::value);

		t.expected = R"(
---
int:   # <Integer>  ⚠️ REQUIRED
)";
	}

	SECTION("with data") {
		using H = testing::Holder<std::string>;

		t.node = Node(Source::make({_{"str", "hypnos"}}));
		t.node.is<Type::Map>().to<H>()
		    | field("str", &H::value);

		t.expected = R"(
---
str: hypnos
)";
	}

	SECTION("default value") {
		using H = testing::Holder<std::vector<int>>;

		t.node.is<Type::Map>().to<H>()
		        | field("list", &H::value)
		    || H{{2, 3, 5, 7, 11}};

		t.expected = R"(
---
list: [2, 3, 5, 7, 11]
)";
	}

	SECTION("of StructuredProp") {
		using I = testing::Holder<int>;
		using H = testing::Holder<I>;

		t.node.is<Type::Map>().to<H>()
		    | (field("foo", &H::value)
		       | field("bar", &I::value));

		t.expected = R"(
---
foo: 
  bar:   # <Integer>  ⚠️ REQUIRED
)";
	}

	t.done();
}

TEST_CASE("MonoListProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	ReportTester t;
	t.node = Node(Source::make({2, 3, 5}));

	auto describer = t.node.is<Type::List>().of<Type::Int>();

	SECTION("no data") {
		t.node = Node(Source::null());
		t.node.is<Type::List>().of<Type::Int>();

		t.expected = R"(
---
# -   # <Integer>
# - ...
)";
	}

	SECTION("no constraints") {
		t.expected = R"(
---
[2, 3, 5]
)";
	}

	SECTION("default value") {
		t.node = Node(Source::null());
		t.node.is<Type::List>().of<Type::Int>().defaultValue({2, 4, 8});

		t.expected = R"(
---
# [2, 4, 8]
)";
	}

	SECTION("size") {
		describer.size(2 < x <= 5);

		t.expected = R"(
---
# • { |x| ∈ W | 2 < |x| ≤ 5 }
[2, 3, 5]
)";
	}

	SECTION("of MonoListProp") {
		SECTION("no data") {
			t.node = Node(Source::null());
			t.node.is<Type::List>().of(prop<Type::List>().of<Type::Int>());

			t.expected = R"(
---
# - 
#   # -   # <Integer>
#   # - ...
# - ...
)";
		}

		SECTION("with data") {
			t.node = Node(Source::make({
			    {1, 2, 3},
			    {4, 5, 6},
			    {7, 8, 9},
			}));
			t.node.is<Type::List>().of(prop<Type::List>().of<Type::Int>());

			t.expected = R"(
---
- [1, 2, 3]
- [4, 5, 6]
- [7, 8, 9]
)";
		}
	}

	SECTION("of MonoMapProp") {
		SECTION("no data") {
			t.node = Node(Source::null());
			t.node.is<Type::List>().of(prop<Type::Map>().of<Type::Int>());

			t.expected = R"(
---
# - 
#   # key:   # <Integer>
#   # ...
# - ...
)";
		}

		SECTION("with data") {
			t.node = Node(Source::make({
			    {_{"a", 1}, _{"b", 2}},
			    {_{"c", 3}, _{"d", 4}},
			}));
			t.node.is<Type::List>().of(prop<Type::Map>().of<Type::Int>());

			t.expected = R"(
---
- 
  a: 1
  b: 2
- 
  c: 3
  d: 4
)";
		}
	}

	t.done();
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

	ReportTester t;

	std::string expected;

	SECTION("no data") {
		t.expected = R"(
---
dish: 
  flag:   # <Bool>  ⚠️ REQUIRED
  int:   # <Integer>  ⚠️ REQUIRED
  num:   # <Number>  ⚠️ REQUIRED
  str:   # <String>  ⚠️ REQUIRED
  extrinsics: 
    # key: 
    #   # -   # <Number>
    #   # - ...
    # ...
)";
	}

	t.node["dish"].is<Type::Map>().to<Enchilada>()
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

	t.done();
}

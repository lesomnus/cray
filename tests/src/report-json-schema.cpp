#include <optional>
#include <sstream>
#include <string>
#include <utility>

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
		cray::report::asJsonSchema(rst, this->node);
		rst << std::endl;

		testing::requireReportEq(this->expected, std::move(rst).str());
	}

	cray::Node  node = cray::Node(cray::Source::null());
	std::string expected;

   private:
	bool is_done_ = false;
};

TEST_CASE("Annotation") {
	using namespace cray;

	ReportTester t;
	Annotation   annotation;

	SECTION("no annotation") {
		t.expected = R"(
{
	"type": "integer"
}
)";
	}

	SECTION("all") {
		annotation = Annotation{
		    .title         = "Title",
		    .description   = "Description",
		    .is_deprecated = true,
		};

		t.expected = R"(
{
	"type": "integer",
	"title": "Title",
	"description": "Description",
	"deprecated": true
}
)";
	}

	t.node.is<Type::Int>(annotation);
	t.done();
}

TEST_CASE("NilProp") {
	using namespace cray;

	ReportTester t;

	auto desc = t.node.is<Type::Nil>();

	SECTION("no constraints") {
		t.expected = R"(
{
	"type": "null"
}
)";
	}

	t.done();
}

TEST_CASE("BoolProp") {
	using namespace cray;

	ReportTester t;

	auto desc = t.node.is<Type::Bool>();

	SECTION("no constraints") {
		t.expected = R"(
{
	"type": "boolean"
}
)";
	}

	t.done();
}

TEST_CASE("IntProp") {
	using namespace cray;

	ReportTester t;

	auto desc = t.node.is<Type::Int>();

	SECTION("no constraints") {
		t.expected = R"(
{
	"type": "integer"
}
)";
	}

	SECTION("multipleOf") {
		desc.mutipleOf(42);

		t.expected = R"(
{
	"type": "integer",
	"multipleOf": 42
}
)";
	}

	SECTION("interval") {
		desc.interval(3 < x <= 7);

		t.expected = R"(
{
	"type": "integer",
	"minimum": 3,
	"exclusiveMinimum": true,
	"maximum": 7
}
)";
	}

	t.done();
}

TEST_CASE("NumProp") {
	using namespace cray;

	ReportTester t;

	auto desc = t.node.is<Type::Num>();

	SECTION("no constraints") {
		t.expected = R"(
{
	"type": "number"
}
)";
	}

	SECTION("multipleOf") {
		desc.mutipleOf(0.03);

		t.expected = R"(
{
	"type": "number",
	"multipleOf": 0.03
}
)";
	}

	SECTION("interval") {
		desc.interval(2.718 <= x < 3.14);

		t.expected = R"(
{
	"type": "number",
	"minimum": 2.718,
	"maximum": 3.14,
	"exclusiveMaximum": true
}
)";
	}

	t.done();
}

TEST_CASE("StrProp") {
	using namespace cray;

	ReportTester t;

	auto desc = t.node.is<Type::Str>();

	SECTION("no constraints") {
		t.expected = R"(
{
	"type": "string"
}
)";
	}

	SECTION("oneOf") {
		desc.oneOf({"foo", "bar", "baz"});

		t.expected = R"(
{
	"type": "string",
	"enum": [
		"foo",
		"bar",
		"baz"
	]
}
)";
	}

	SECTION("length") {
		desc.length(3 <= x < 5);

		t.expected = R"(
{
	"type": "string",
	"minLength": 3,
	"maxLength": 4
}
)";
	}

	t.done();
}

TEST_CASE("MonoMapProp") {
	using namespace cray;

	ReportTester t;

	auto desc = t.node.is<Type::Map>().of<Type::Int>();

	SECTION("no constraints") {
		t.expected = R"(
{
	"type": "object",
	"additionalProperties": {
		"type": "integer"
	}
}
)";
	}

	t.done();
}

TEST_CASE("PolyMapProp") {
	using namespace cray;

	ReportTester t;

	SECTION("no constraints") {
		t.node["int"].is<Type::Int>();
		t.node["str"].is<Type::Str>();

		t.expected = R"(
{
	"type": "object",
	"properties": {
		"int": {
			"type": "integer"
		},
		"str": {
			"type": "string"
		}
	}
}
)";
	}

	SECTION("required keys") {
		t.node["int"].is<Type::Int>();
		t.node["str"].is<Type::Str>().get();

		t.expected = R"(
{
	"type": "object",
	"required": [
		"str"
	],
	"properties": {
		"int": {
			"type": "integer"
		},
		"str": {
			"type": "string"
		}
	}
}
)";
	}

	t.done();
}

TEST_CASE("StructuredMapProp") {
	using namespace cray;

	ReportTester t;

	SECTION("no constraints") {
		using H = testing::Holder<int>;
		t.node.is<Type::Map>().to<H>()
		    | field("int", &H::value);

		t.expected = R"(
{
	"type": "object",
	"required": [
		"int"
	],
	"properties": {
		"int": {
			"type": "integer"
		}
	}
}
)";
	}

	SECTION("required keys") {
		using H = testing::Holder<std::optional<int>>;
		t.node.is<Type::Map>().to<H>()
		    | field("int", &H::value);

		t.expected = R"(
{
	"type": "object",
	"properties": {
		"int": {
			"type": "integer"
		}
	}
}
)";
	}

	t.done();
}

TEST_CASE("MonoListProp") {
	using namespace cray;

	ReportTester t;

	auto desc = t.node.is<Type::List>().of<Type::Int>();

	SECTION("no constraints") {
		t.expected = R"(
{
	"type": "array",
	"items": {
		"type": "integer"
	}
}
)";
	}

	t.done();
}

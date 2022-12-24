#include <iostream>
#include <sstream>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <cray/detail/props.hpp>
#include <cray/node.hpp>
#include <cray/report.hpp>
#include <cray/source.hpp>

TEST_CASE("MonoMapProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	Node node(Source::make({
	    _{"a", 3},
	    _{"b", 6},
	    _{"c", 9},
	}));

	node.is<Type::Map>().of<Type::Int>(Annotation{
	    .title       = "Title",
	    .description = "Description",
	});

	std::stringstream rst;
	report::asYaml(rst, node);

	REQUIRE(
	    R"(# Title
# Description
a: 3
b: 6
c: 9
)" == std::move(rst).str());
}

#include <string>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

#include <cray/detail/ordered_map.hpp>

TEST_CASE("OrderedMap") {
	std::vector<std::string> const order{"a", "b", "c"};

	cray::detail::OrderedMap<std::string, int> map{
	    {"a", 36},
	    {"b", 21},
	    {"a", 42},
	};

	REQUIRE(2 == map.size());
	REQUIRE(42 == map.at("a"));
	REQUIRE(21 == map.at("b"));
	{
		std::size_t i = 0;
		for(auto const& [k, v]: map) {
			REQUIRE(order[i] == k);
			REQUIRE(map.at(k) == v);
			++i;
		}
		REQUIRE(2 == i);
	}

	map["c"] = 13;
	map["b"] = 31;

	REQUIRE(3 == map.size());
	REQUIRE(42 == map.at("a"));
	REQUIRE(31 == map.at("b"));
	REQUIRE(13 == map.at("c"));
	{
		std::size_t i = 0;
		for(auto const& [k, v]: map) {
			REQUIRE(order[i] == k);
			REQUIRE(map.at(k) == v);
			++i;
		}
		REQUIRE(3 == i);
	}
}

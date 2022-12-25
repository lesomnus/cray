#include <algorithm>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <cray/detail/ordered_set.hpp>

TEST_CASE("OrderedSet") {
	std::vector<int> const order{2, 3, 5, 7, 11, 13};

	cray::detail::OrderedSet<int> set{2, 3, 5, 7, 11, 3, 5};
	REQUIRE(5 == set.size());
	REQUIRE(set.end() != set.find(7));
	REQUIRE(7 == *set.find(7));
	REQUIRE(set.end() != set.find(11));
	REQUIRE(11 == *set.find(11));
	{
		std::size_t i = 0;
		for(auto v: set) {
			REQUIRE(order[i] == v);
			++i;
		}

		REQUIRE(5 == i);
	}

	set.insert(11);
	set.insert(13);
	REQUIRE(6 == set.size());
	REQUIRE(set.end() != set.find(13));
	REQUIRE(13 == *set.find(13));
	{
		std::size_t i = 0;
		for(auto v: set) {
			REQUIRE(order[i] == v);
			++i;
		}

		REQUIRE(6 == i);
	}

	set.erase(std::remove(set.begin(), set.end(), 11), set.end());
	REQUIRE(5 == set.size());
	REQUIRE(set.end() == set.find(11));
}

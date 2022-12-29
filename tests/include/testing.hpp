#include <algorithm>
#include <string>

#include <catch2/catch_test_macros.hpp>

namespace testing {

void renderWhitespaces(std::string& value) {
	std::replace(value.begin(), value.end(), ' ', '.');
}

void requireReportEq(std::string expected, std::string actual) {
	renderWhitespaces(expected);
	renderWhitespaces(actual);

	CAPTURE(expected.size());
	CAPTURE(actual.size());

	REQUIRE(expected == actual);
}

}  // namespace testing

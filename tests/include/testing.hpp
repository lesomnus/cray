#include <algorithm>
#include <regex>
#include <string>

#include <catch2/catch_test_macros.hpp>

namespace testing {

void renderWhitespaces(std::string& value) {
	value = std::regex_replace(value, std::regex(" "), "·");
	value = std::regex_replace(value, std::regex("\n"), "⤶\n");
	value = std::regex_replace(value, std::regex("\t"), "⤚⇥");
}

void requireReportEq(std::string expected, std::string actual) {
	renderWhitespaces(expected);
	renderWhitespaces(actual);

	CAPTURE(expected.size());
	CAPTURE(actual.size());

	REQUIRE(expected == actual);
}

template<typename T>
struct Holder {
	T value;
};

}  // namespace testing

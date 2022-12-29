#include <sstream>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <cray.hpp>

struct Step {
	std::string name;
	std::string run;
};

struct Job {
	std::vector<std::string> runs_on;
	std::vector<Step>        steps;
};

TEST_CASE("example-yaml") {
	constexpr auto* data = R"(
name: Test
on: [push, pull_request]
jobs:
  test:
    runs-on: [ubuntu-20.04]
    steps:
      - name: Test
        run: build && test
)";

	std::stringstream in(data);

	using namespace cray;
	Node doc(load::fromYaml(in));

	auto const name = doc["name"].as<std::string>();
	auto const on   = doc["on"].is<Type::List>().of(
        prop<Type::Str>().oneOf({
            "push",
            "pull_request",
            "workflow_dispatch",
        }));

	auto step =
	    prop<Type::Map>().to<Step>()
	    | field("name", &Step::name)
	    | field("run", &Step::run);

	auto const jobs = doc["jobs"].is<Type::Map>().of(
	    prop<Type::Map>().to<Job>()
	    | field("runs-on", &Job::runs_on)
	    | field("steps", &Job::steps, step));

	REQUIRE(doc.ok());

	std::stringstream out;
	out << std::endl;
	report::asYaml(out, doc);
	out << std::endl;

	constexpr auto* expected = R"(
name: Test
on: [push, pull_request]
jobs: 
  test: 
    runs-on: [ubuntu-20.04]
    steps: 
      - 
        name: Test
        run: build && test
)";
	REQUIRE(expected == std::move(out).str());
}

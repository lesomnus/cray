#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <cray.hpp>

#include "testing.hpp"

struct Step {
	std::string name;
	std::string run;
};

struct Job {
	std::vector<std::string> runs_on;
	std::vector<Step>        steps;
};

TEST_CASE("example-yaml") {
	using namespace cray;

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

	Node doc(load::fromYaml(in));

	auto const name = doc["name"].as<std::string>(Annotation{
	    .title       = "Name",
	    .description = "The name of your workflow.",
	});

	auto const run_name = doc["run-name"].as<std::optional<std::string>>(Annotation{
	    .title       = "Run Name",
	    .description = "The name for workflow runs generated from the workflow.",
	});

	auto events = prop<Type::Str>().oneOf({
	    "push",
	    "pull_request",
	    "workflow_dispatch",
	});

	auto const on = doc["on"].is<Type::List>().of(events).get();

	auto step =
	    prop<Type::Map>().to<Step>()
	    | field("name", &Step::name)
	    | field("run", &Step::run);

	auto job =
	    prop<Type::Map>().to<Job>()
	    | field("runs-on", &Job::runs_on)
	    | field("steps", &Job::steps, step);

	auto const jobs = doc["jobs"].is<Type::Map>().of(job).get();

	REQUIRE(doc.ok());

	std::stringstream out;
	out << std::endl;
	report::asYaml(out, doc);
	out << std::endl;

	constexpr auto* expected = R"(
# Name
# | The name of your workflow.
name: Test

# Run Name
# | The name for workflow runs generated from the workflow.
run-name:   # <String>

on: 
  # • push | pull_request | workflow_dispatch
  [push, pull_request]
jobs: 
  test: 
    runs-on: [ubuntu-20.04]
    steps: 
      - 
        name: Test
        run: build && test
)";

	testing::requireReportEq(expected, std::move(out).str());

	REQUIRE("Test" == name);
	REQUIRE(!run_name.has_value());

	REQUIRE(2 == on.size());
	REQUIRE("push" == on[0]);
	REQUIRE("pull_request" == on[1]);

	REQUIRE(1 == jobs.size());
	REQUIRE(jobs.contains("test"));

	auto const job_test = jobs.at("test");
	REQUIRE(job_test.runs_on.size());
	REQUIRE("ubuntu-20.04" == job_test.runs_on[0]);
	REQUIRE(1 == job_test.steps.size());

	auto const step_test = job_test.steps[0];
	REQUIRE("Test" == step_test.name);
	REQUIRE("build && test" == step_test.run);
}

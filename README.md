# CRay

[![test](https://github.com/lesomnus/cray/actions/workflows/test.yaml/badge.svg)](https://github.com/lesomnus/cray/actions/workflows/test.yaml)
[![codecov](https://codecov.io/gh/lesomnus/cray/branch/main/graph/badge.svg?token=Zw14Luij1P)](https://codecov.io/gh/lesomnus/cray)

Access configs in a structured manner with validation and automatically generate documented configs.

CRay creates a schema for your data by tracking how you access it. You don't have to keep checking each value in the middle of your code to see if it satisfies the condition you want. Validate documents at once with an automatically generated schema, and see at a glance what values went wrong.



## Support

### Loaders

- YAML (powered by [jbeder/yaml-cpp](https://github.com/jbeder/yaml-cpp))

### Reporters

- YAML
- JSON Schema



## CMake Integration
```cmake
include(FetchContent)
FetchContent_Declare(
  CRay
  GIT_REPOSITORY https://github.com/lesomnus/cray.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(CRay)

...

add_library(foo ...)
...
target_link_libraries(
	foo PRIVATE
		CRay::CRay
		CRay::yaml  # YAML loader support
)
```

Note that `CRay::yaml` is enabled only if the `yaml-cpp` package is available.


## Example

You may want to read what kind of [describers](docs/describers.md) are available.

```cpp
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <cray.hpp>

struct Step {
	std::string name;
	std::string run;
};

struct Job {
	std::vector<std::string> runs_on;
	std::vector<Step>        steps;
};

int main(int argc, char*[] argv) {
	using namespace cray;
	
	Node doc(load::fromYaml("workflow.yaml"));

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

	std::ofstream out("workflow.yaml");
	report::asYaml(out, doc);
	
	std::ofstream out("workflow.schema.json");
	report::asJsonSchema(out, doc);

	if(!doc.ok()){
		// There is a field that does not satisfy the condition or has the wrong type.
		return -1;
	}

	// name == "Test"
	// jobs["test"].steps[0].run == "build && test"
	// ...
}
```

Consider original `workflow.yaml`:
```yaml
name: Test
on: [push, pull_request]
jobs:
  test:
    runs-on: [ubuntu-20.04]
    steps:
      - name: Test
        run: build && test
``` 

Will be:
```yaml
# Name
# | The name of your workflow.
name: Test

# Run Name
# | The name for workflow runs generated from the workflow.
run-name:   # <String>

on: 
  # ??? push | pull_request | workflow_dispatch
  [push, pull_request]
jobs: 
  test: 
    runs-on: [ubuntu-20.04]
    steps: 
      - 
        name: Test
        run: build && test
```

And you can have JSON schema `workflow.schema.json`:

> This is a summarized result. Full results can be found at [tests/src/example-report.cpp](tests/src/example-report.cpp).

```json
{
	"type": "object",
	"required": [
		"name",
		"on",
		"jobs"
	],
	"properties": {
		"name": {
			"type": "string",
			"title": "Name",
			"description": "The name of your workflow."
		},
		"run-name": {
			"type": "string",
			"title": "Run Name",
			"description": "The name for workflow runs generated from the workflow."
		},
		"on": {
			"type": "array",
			"items": {
				"type": "string",
				"enum": [
					"push",
					"pull_request",
					"workflow_dispatch"
				]
			}
		},
		"jobs": {
			...
		}
	}
}
```

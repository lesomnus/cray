# CRay

Access configs in a structured manner with validation and automatically generate documented configs.

## Example

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

	std::ofstream out("workflow.yaml");
	report::asYaml(out, node);
	
	std::ofstream out("workflow.schema.yaml");
	report::asJsonSchema(out, node);

	if(!node.ok()){
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
  # • push | pull_request | workflow_dispatch
  [push, pull_request]
jobs: 
  test: 
    runs-on: [ubuntu-20.04]
    steps: 
      - 
        name: Test
        run: build && test
```

And you can have JSON schema:

> This is a short result. Full results can be found at [tests/src/example-report.cpp](tests/src/example-report.cpp).

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

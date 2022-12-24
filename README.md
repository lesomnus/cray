# CRay

Access configs in a structured manner and automatically generate documented configs.

## Example

```cpp
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

#include <cray.hpp>

struct Addr {
	std::string host;
	int         port;
};

int main(int argc, char*[] argv) {
	using namespace cray;
	Node node(Source::fromYaml("request.yaml"));

	// operator||(...) sets default value.

	auto addr =
	    node["addr"].is<Type::Map>().of<Addr>(Annotation{
	        .title       = "Address",
	        .description = "Address of the server to connect to.",
	    })
	        | filed("host", &Addr::host)
	        | field("port", &Addr::port)
	    || Adder{
	        .host = "localhost",
	        .port = 0,
	    };

	auto method = node["method"].is<Type::String>().oneOf({"GET", "POST", "PUT"}) || "GET";
	auto headers =
	    node["headers"].is<Type::Map>(Annotation{.title = "HTTP Headers"}).of<Type::Str>()
	    || std::unordered_map<std::string, std::string>{
	        {"Cache-Control", "max-age=0"},
	    };

	std::ofstream out("request.yaml");
	report::asYaml(out, node);

	if(!node.ok()){
		// There is a field that does not satisfy the condition or has the wrong type.
		return -1;
	}

	// ...
}
```

Consider original `request.yaml`:
```yaml
addr:
  host: http://github.com/lesmonus
``` 

Will be:
```yaml
# Address
# | Address of the server to connect to.
addr:
  host: http://github.com/lesmonus
  port: # 0

method: # GET

# HTTP Headers
headers:
  # Cache-Control: max-age=0
```

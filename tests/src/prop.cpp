#include <optional>
#include <string>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

#include <cray/detail/props/int.hpp>
#include <cray/detail/props/list.hpp>
#include <cray/detail/props/map.hpp>
#include <cray/detail/props/str.hpp>
#include <cray/node.hpp>

struct Nested {
	int value;
};

struct Profile {
	std::string name;
	int         age;

	Nested nested;

	std::optional<unsigned int> id;
};

template<typename T, typename U>
bool be(T answer, U const& convertible) {
	return answer == static_cast<T>(convertible);
}

TEST_CASE("PropOf") {
	using namespace cray;
	using namespace cray::detail;

	STATIC_REQUIRE(std::is_same_v<IntProp, PropOf<Type::Int>>);
	STATIC_REQUIRE(std::is_same_v<StrProp, PropOf<Type::Str>>);
}

TEST_CASE("PropFor") {
	using namespace cray;
	using namespace cray::detail;

	STATIC_REQUIRE(std::is_same_v<IntProp, PropFor<short>>);
	STATIC_REQUIRE(std::is_same_v<IntProp, PropFor<int>>);
	STATIC_REQUIRE(std::is_same_v<IntProp, PropFor<long>>);
	STATIC_REQUIRE(std::is_same_v<IntProp, PropFor<long long>>);
	STATIC_REQUIRE(std::is_same_v<IntProp, PropFor<unsigned short>>);
	STATIC_REQUIRE(std::is_same_v<IntProp, PropFor<unsigned>>);
	STATIC_REQUIRE(std::is_same_v<IntProp, PropFor<unsigned long>>);
	STATIC_REQUIRE(std::is_same_v<IntProp, PropFor<unsigned long long>>);

	STATIC_REQUIRE(std::is_same_v<StrProp, PropFor<std::string>>);
}

TEST_CASE("getProp") {
	using namespace cray;

	auto desc = prop<Type::Int>();

	auto prop = detail::getProp(desc);
	STATIC_REQUIRE(std::is_same_v<std::shared_ptr<detail::PropOf<Type::Int>>, decltype(prop)>);
	REQUIRE(nullptr != prop);
	REQUIRE(2 == prop.use_count());

	auto moved_prop = detail::getProp(std::move(desc));
	REQUIRE(nullptr != moved_prop);
	REQUIRE(2 == moved_prop.use_count());
}

TEST_CASE("IntProp::Describer") {
	using namespace cray;

	Node node(Source::make(42));

	SECTION("::defaultValue") {
		Node null_node(Source::null());
		null_node.is<Type::Int>().get();
		REQUIRE(!null_node.ok());

		auto value = null_node.is<Type::Int>() || 42;
		REQUIRE(42 == value);
		REQUIRE(null_node.ok());
	}

	SECTION("::multipleOf") {
		node.is<Type::Int>().mutipleOf(3);
		REQUIRE(node.ok());

		node.is<Type::Int>().mutipleOf(4);
		REQUIRE(!node.ok());
	}

	SECTION("::interval") {
		node.is<Type::Int>().interval(0 < x);
		REQUIRE(node.ok());

		node.is<Type::Int>().interval(x < 0);
		REQUIRE(!node.ok());
	}

	SECTION("::withClamp") {
		auto const desc = node.is<Type::Int>().interval(x < 13);
		REQUIRE(!node.ok());

		desc.withClamp();
		REQUIRE(node.ok());
		REQUIRE(13 > static_cast<int>(desc));
	}

	SECTION("::opt") {
		Node null_node(Source::null());

		REQUIRE(!null_node.is<Type::Int>().opt().has_value());
		REQUIRE(null_node.ok());

		REQUIRE(42 == null_node.is<Type::Int>().defaultValue(42).opt().value());
	}

	SECTION("::get") {
		REQUIRE(42 == node.is<Type::Int>().get());

		Node null_node(Source::null());
		null_node.is<Type::Int>();
		REQUIRE(null_node.ok());

		null_node.is<Type::Int>().get();
		REQUIRE(!null_node.ok());
	}

	SECTION("::operator T") {
		auto desc = node.is<Type::Int>();
		REQUIRE(be<short>(42, desc));
		REQUIRE(be<int>(42, desc));
		REQUIRE(be<long>(42, desc));
		REQUIRE(be<long long>(42, desc));
		REQUIRE(be<unsigned short>(42, desc));
		REQUIRE(be<unsigned>(42, desc));
		REQUIRE(be<unsigned long>(42, desc));
		REQUIRE(be<unsigned long long>(42, desc));
	}
}

TEST_CASE("StrProp::Describer") {
	using namespace cray;

	Node node(Source::make("hypnos"));

	SECTION("::defaultValue") {
		Node null_node(Source::null());
		null_node.is<Type::Str>().get();
		REQUIRE(!null_node.ok());

		auto value = null_node.is<Type::Str>() || "hypnos";
		REQUIRE("hypnos" == value);
		REQUIRE(null_node.ok());
	}

	SECTION("::interval") {
		node.is<Type::Str>().length(0 < x);
		REQUIRE(node.ok());

		node.is<Type::Str>().length(x < 0);
		REQUIRE(!node.ok());
	}

	SECTION("::opt") {
		Node null_node(Source::null());

		REQUIRE(!null_node.is<Type::Str>().opt().has_value());
		REQUIRE(null_node.ok());

		REQUIRE("hypnos" == null_node.is<Type::Str>().defaultValue("hypnos").opt().value());
	}

	SECTION("::get") {
		REQUIRE("hypnos" == node.is<Type::Str>().get());

		Node null_node(Source::null());
		null_node.is<Type::Str>();
		REQUIRE(null_node.ok());

		null_node.is<Type::Str>().get();
		REQUIRE(!null_node.ok());
	}

	SECTION("::operator T") {
		auto desc = node.is<Type::Str>();
		REQUIRE(be<std::string>("hypnos", desc));
	}
}

TEST_CASE("PolyMapProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	Node node(Source::make({
	    _{"a", {_{"b", {_{"c", 42}}}}},
	}));

	auto const value = node["a"]["b"]["c"].is<Type::Int>().get();
	REQUIRE(42 == value);
}

TEST_CASE("MapProp::Describer") {
	using namespace cray;

	Node node(Source::null());

	node.is<Type::Map>().of(prop<Type::Int>().mutipleOf(42)).containing({"a", "b", "c"});
	node.is<Type::Map>().to<Profile>() | field("age", &Profile::age);
}

TEST_CASE("MonoMapProp::Describer") {
	using namespace cray;

	Node node(Source::make({std::pair{"answer", 42}}));

	SECTION("::defaultValue") {
		Node null_node(Source::null());
		null_node.is<Type::Map>().of<Type::Int>().get();
		REQUIRE(!null_node.ok());

		auto const value = null_node.is<Type::Map>().of<Type::Int>().withDefault({{"answer", 42}});
		REQUIRE(null_node.ok());
		REQUIRE(1 == value.size());
		REQUIRE(42 == value.at("answer"));
	}

	SECTION("::containing") {
		node.is<Type::Map>().of<Type::Int>().containing({"foo"});
		REQUIRE(!node.ok());

		node.is<Type::Map>().of<Type::Int>().containing({"answer", "foo"});
		REQUIRE(!node.ok());

		node.is<Type::Map>().of<Type::Int>().containing({"answer"});
		REQUIRE(node.ok());
	}

	SECTION("::opt") {
		Node null_node(Source::null());

		REQUIRE(!null_node.is<Type::Map>().of<Type::Int>().opt().has_value());
		REQUIRE(null_node.ok());

		auto const value = null_node.is<Type::Map>().of<Type::Int>().defaultValue({{"answer", 42}}).opt().value();
		REQUIRE(1 == value.size());
		REQUIRE(42 == value.at("answer"));
	}

	SECTION("::get") {
		auto const value = node.is<Type::Map>().of<Type::Int>().get();
		REQUIRE(node.ok());
		REQUIRE(1 == value.size());
		REQUIRE(42 == value.at("answer"));
	}
}

TEST_CASE("StructuredProp::Describer") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	Node node(Source::make({
	    _{"name", "hypnos"},
	    _{"age", 42},
	    _{
	        "nested",
	        {_{"value", 21}}},
	}));

	SECTION("::defaultValue") {
		Node null_node(Source::null());
		null_node.is<Type::Map>().to<Profile>() | field("age", &Profile::age) && get;
		REQUIRE(!null_node.ok());

		auto const profile = null_node.is<Type::Map>().to<Profile>() | field("age", &Profile::age) || Profile{.age = 42};
		REQUIRE(null_node.ok());
		REQUIRE(42 == profile.age);
	}

	SECTION("optional field") {
		node.is<Type::Map>().to<Profile>() | field("id", &Profile::id) && get;
		REQUIRE(node.ok());
	}

	SECTION("query get") {
		auto const profile =
		    node.is<Type::Map>().to<Profile>()
		        | field("name", &Profile::name)
		        | field("age", &Profile::age)
		        | field("id", &Profile::id)
		    && get;

		REQUIRE(node.ok());
		REQUIRE("hypnos" == profile.name);
		REQUIRE(42 == profile.age);
		REQUIRE(!profile.id.has_value());
	}

	SECTION("nested structured field") {
		auto const profile =
		    node.is<Type::Map>().to<Profile>()
		        | (field("nested", &Profile::nested)
		           | field("value", &Nested::value))
		    && get;

		REQUIRE(node.ok());
		REQUIRE(21 == profile.nested.value);
	}
}

TEST_CASE("ListProp::Describer") {
	using namespace cray;

	Node node(Source::null());

	node.is<Type::List>().of(prop<Type::Int>().mutipleOf(42)).size(2 < x);
}

TEST_CASE("MonoListProp::Describer") {
	using namespace cray;

	Node node(Source::make({3, 5, 13, 21, 42}));

	SECTION("::defaultValue") {
		Node null_node(Source::null());
		null_node.is<Type::List>().of<Type::Int>().get();
		REQUIRE(!null_node.ok());

		auto const value = null_node.is<Type::List>().of<Type::Int>().withDefault({7, 42, 53});
		REQUIRE(null_node.ok());
		REQUIRE(3 == value.size());
		REQUIRE(42 == value.at(1));
	}

	SECTION("::size") {
		node.is<Type::List>().of<Type::Int>().size(x < 3);
		REQUIRE(!node.ok());

		node.is<Type::List>().of<Type::Int>().size(42 < x);
		REQUIRE(!node.ok());

		node.is<Type::List>().of<Type::Int>().size(3 < x < 8);
		REQUIRE(node.ok());
	}

	SECTION("::opt") {
		Node null_node(Source::null());

		REQUIRE(!null_node.is<Type::List>().of<Type::Int>().opt().has_value());
		REQUIRE(null_node.ok());

		auto const value = null_node.is<Type::List>().of<Type::Int>().defaultValue({7, 42, 53}).opt().value();
		REQUIRE(3 == value.size());
		REQUIRE(42 == value.at(1));
	}

	SECTION("::get") {
		auto const value = node.is<Type::List>().of<Type::Int>().get();
		REQUIRE(node.ok());
		REQUIRE(5 == value.size());
		REQUIRE(13 == value.at(2));
	}
}

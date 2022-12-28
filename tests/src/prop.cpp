#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <cray/detail/props.hpp>
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

template<typename T>
struct Holder {
	T value;
};

template<typename T, typename U>
bool be(T answer, U const& convertible) {
	return answer == static_cast<T>(convertible);
}

template<cray::Type T>
void scalarCodecPropTest(cray::StorageOf<T> const default_value) {
	using namespace cray;

	SECTION("::defaultValue") {
		Node null_node(Source::null());
		null_node.is<T>().get();
		REQUIRE(!null_node.ok());

		auto value = null_node.is<T>() || default_value;
		REQUIRE(default_value == value);
		REQUIRE(null_node.ok());
	}

	SECTION("::encodeDefaultValueInto") {
		auto source = Source::make({1, 2, 3});
		REQUIRE(!source->is(T));

		auto p = detail::getProp(prop<T>() || default_value);
		p->encodeDefaultValueInto(*source);
		REQUIRE(source->is(T));

		StorageOf<T> v;
		REQUIRE(source->get(v));
		REQUIRE(default_value == v);
	}

	SECTION("::opt") {
		Node null_node(Source::null());

		REQUIRE(!null_node.is<T>().opt().has_value());
		REQUIRE(null_node.ok());

		REQUIRE(default_value == null_node.is<T>().defaultValue(default_value).opt().value());
	}

	SECTION("::get") {
		Node node(Source::make(default_value));
		REQUIRE(default_value == node.is<T>().get());

		Node null_node(Source::null());
		null_node.is<T>();
		REQUIRE(null_node.ok());

		null_node.is<T>().get();
		REQUIRE(!null_node.ok());
	}
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

	STATIC_REQUIRE(std::is_same_v<NilProp, PropFor<std::nullptr_t>>);
	STATIC_REQUIRE(std::is_same_v<BoolProp, PropFor<bool>>);

	STATIC_REQUIRE(std::is_same_v<BasicIntProp<short>, PropFor<short>>);
	STATIC_REQUIRE(std::is_same_v<BasicIntProp<int>, PropFor<int>>);
	STATIC_REQUIRE(std::is_same_v<BasicIntProp<long>, PropFor<long>>);
	STATIC_REQUIRE(std::is_same_v<BasicIntProp<long long>, PropFor<long long>>);
	STATIC_REQUIRE(std::is_same_v<BasicIntProp<unsigned short>, PropFor<unsigned short>>);
	STATIC_REQUIRE(std::is_same_v<BasicIntProp<unsigned int>, PropFor<unsigned int>>);
	STATIC_REQUIRE(std::is_same_v<BasicIntProp<unsigned long>, PropFor<unsigned long>>);
	STATIC_REQUIRE(std::is_same_v<BasicIntProp<unsigned long long>, PropFor<unsigned long long>>);

	STATIC_REQUIRE(std::is_same_v<BasicNumProp<float>, PropFor<float>>);
	STATIC_REQUIRE(std::is_same_v<BasicNumProp<double>, PropFor<double>>);
	STATIC_REQUIRE(std::is_same_v<BasicNumProp<long double>, PropFor<long double>>);

	STATIC_REQUIRE(std::is_same_v<StrProp, PropFor<std::string>>);

	// clang-format off
	STATIC_REQUIRE(std::is_same_v<MonoMapPropOf<PropFor<std::nullptr_t>>, PropFor<std::unordered_map<std::string, std::nullptr_t>>>);
	STATIC_REQUIRE(std::is_same_v<MonoMapPropOf<PropFor<bool>>,           PropFor<std::unordered_map<std::string, bool>>>);
	STATIC_REQUIRE(std::is_same_v<MonoMapPropOf<PropFor<int>>,            PropFor<std::unordered_map<std::string, int>>>);
	STATIC_REQUIRE(std::is_same_v<MonoMapPropOf<PropFor<double>>,         PropFor<std::unordered_map<std::string, double>>>);
	STATIC_REQUIRE(std::is_same_v<MonoMapPropOf<PropFor<std::string>>,    PropFor<std::unordered_map<std::string, std::string>>>);

	STATIC_REQUIRE(std::is_same_v<MonoListPropOf<PropFor<std::nullptr_t>>, PropFor<std::vector<std::nullptr_t>>>);
	STATIC_REQUIRE(std::is_same_v<MonoListPropOf<PropFor<bool>>,           PropFor<std::vector<bool>>>);
	STATIC_REQUIRE(std::is_same_v<MonoListPropOf<PropFor<int>>,            PropFor<std::vector<int>>>);
	STATIC_REQUIRE(std::is_same_v<MonoListPropOf<PropFor<double>>,         PropFor<std::vector<double>>>);
	STATIC_REQUIRE(std::is_same_v<MonoListPropOf<PropFor<std::string>>,    PropFor<std::vector<std::string>>>);
	// clang-format on
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

TEST_CASE("NilProp") {
	using namespace cray;

	Node node(Source::make(nullptr));

	SECTION("CodecProp<Type::Nil>") {
		scalarCodecPropTest<Type::Nil>(nullptr);
	}

	SECTION("::operator T") {
		auto desc = node.is<Type::Nil>();
		REQUIRE(be<std::nullptr_t>(nullptr, desc));
	}
}

TEST_CASE("BoolProp") {
	using namespace cray;

	Node node_true(Source::make(true));
	Node node_false(Source::make(false));

	SECTION("CodecProp<Type::Bool>") {
		scalarCodecPropTest<Type::Bool>(true);
		scalarCodecPropTest<Type::Bool>(false);
	}

	SECTION("::operator T") {
		REQUIRE(be<bool>(true, node_true.is<Type::Bool>()));
		REQUIRE(be<bool>(false, node_false.is<Type::Bool>()));
	}
}

TEST_CASE("IntProp") {
	using namespace cray;

	Node node(Source::make(42));

	SECTION("CodecProp<Type::Int>") {
		scalarCodecPropTest<Type::Int>(42);
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

	SECTION("::operator T") {
		auto desc = node.is<Type::Int>();
		REQUIRE(be<short>(42, desc));
		REQUIRE(be<int>(42, desc));
		REQUIRE(be<long>(42, desc));
		REQUIRE(be<long long>(42, desc));
		REQUIRE(be<unsigned short>(42, desc));
		REQUIRE(be<unsigned int>(42, desc));
		REQUIRE(be<unsigned long>(42, desc));
		REQUIRE(be<unsigned long long>(42, desc));
	}
}

TEST_CASE("NumProp") {
	using namespace cray;

	Node node(Source::make(3.14));

	SECTION("CodecProp<Type::Num>") {
		scalarCodecPropTest<Type::Num>(3.14);
	}

	SECTION("::multipleOf") {
		node.is<Type::Num>().mutipleOf(0.02);
		REQUIRE(node.ok());

		node.is<Type::Num>().mutipleOf(0.03);
		REQUIRE(!node.ok());
	}

	SECTION("::interval") {
		node.is<Type::Num>().interval(0 < x);
		REQUIRE(node.ok());

		node.is<Type::Num>().interval(x < 0);
		REQUIRE(!node.ok());
	}

	SECTION("::withClamp") {
		auto const desc = node.is<Type::Num>().interval(x < 2.718);
		REQUIRE(!node.ok());

		desc.withClamp();
		REQUIRE(node.ok());
		REQUIRE(2.718 > static_cast<double>(desc));
	}

	SECTION("::operator T") {
		auto desc = node.is<Type::Num>();
		REQUIRE(be<float>(3.14, desc));
		REQUIRE(be<double>(3.14, desc));
		REQUIRE(be<long double>(3.14, desc));
	}
}

TEST_CASE("StrProp") {
	using namespace cray;

	Node node(Source::make("hypnos"));

	SECTION("CodecProp<Type::Str>") {
		scalarCodecPropTest<Type::Str>("hypnos");
	}

	SECTION("::interval") {
		node.is<Type::Str>().length(0 < x);
		REQUIRE(node.ok());

		node.is<Type::Str>().length(x < 0);
		REQUIRE(!node.ok());
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

TEST_CASE("MapProp") {
	using namespace cray;

	Node node(Source::null());

	node.is<Type::Map>().of(prop<Type::Int>().mutipleOf(42)).containing({"a", "b", "c"});
	node.is<Type::Map>().to<Profile>() | field("age", &Profile::age);
}

TEST_CASE("MonoMapProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	SECTION("::defaultValue") {
		Node null_node(Source::null());
		null_node.is<Type::Map>().of<Type::Int>().get();
		REQUIRE(!null_node.ok());

		auto const value = null_node.is<Type::Map>().of<Type::Int>().withDefault({{"answer", 42}});
		REQUIRE(null_node.ok());
		REQUIRE(1 == value.size());
		REQUIRE(42 == value.at("answer"));
	}

	SECTION("::encodeDefaultValueInto") {
		auto source = Source::make(nullptr);
		REQUIRE(!source->is(Type::Map));

		auto p = detail::getProp(prop<Type::Map>().of<Type::Int>().defaultValue({
		    {"a", 1},
		    {"b", 2},
		    {"c", 3},
		}));
		p->encodeDefaultValueInto(*source);
		REQUIRE(source->is(Type::Map));

		REQUIRE(3 == source->size());

		StorageOf<Type::Int> v;
		REQUIRE((source->next("a")->get(v) && (1 == v)));
		REQUIRE((source->next("b")->get(v) && (2 == v)));
		REQUIRE((source->next("c")->get(v) && (3 == v)));
	}

	SECTION("::containing") {
		Node node(Source::make({_{"answer", 42}}));

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
		Node node(Source::make({_{"answer", 42}}));

		auto const value = node.is<Type::Map>().of<Type::Int>().get();
		REQUIRE(node.ok());
		REQUIRE(1 == value.size());
		REQUIRE(42 == value.at("answer"));
	}

	SECTION("nested") {
		SECTION("::get") {
			Node node(Source::make({
			    _{"A", {_{"a", 1}, _{"b", 2}, _{"c", 3}}},
			    _{"B", {_{"d", 4}, _{"e", 5}, _{"f", 6}}},
			    _{"C", {_{"g", 7}, _{"h", 8}, _{"i", 9}}},
			}));

			std::unordered_map<std::string, std::unordered_map<std::string, StorageOf<Type::Int>>> const expected{
			    {"A", {{"a", 1}, {"b", 2}, {"c", 3}}},
			    {"B", {{"d", 4}, {"e", 5}, {"f", 6}}},
			    {"C", {{"g", 7}, {"h", 8}, {"i", 9}}},
			};

			auto const actual = node.is<Type::Map>().of(prop<Type::Map>().of<Type::Int>()).get();
			REQUIRE(expected == actual);
		}
	}

	SECTION("of MonoList") {
		SECTION("::get") {
			Node node(Source::make({
			    _{"a", {1, 2, 3}},
			    _{"b", {4, 5, 6}},
			    _{"c", {7, 8, 9}},
			}));

			std::unordered_map<std::string, std::vector<StorageOf<Type::Int>>> const expected{
			    {"a", {1, 2, 3}},
			    {"b", {4, 5, 6}},
			    {"c", {7, 8, 9}},
			};

			auto const actual = node.is<Type::Map>().of(prop<Type::List>().of<Type::Int>()).get();
			REQUIRE(expected == actual);
		}
	}
}

TEST_CASE("StructuredProp") {
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

	SECTION("::encodeDefaultValueInto") {
		auto source = Source::make(nullptr);
		REQUIRE(!source->is(Type::Map));

		auto p = detail::getProp(
		    prop<Type::Map>().to<Profile>()
		        | field("name", &Profile::name)
		        | field("age", &Profile::age)
		    || Profile{
		        .name = "hypnos",
		        .age  = 42,
		    });
		p->encodeDefaultValueInto(*source);
		REQUIRE(source->is(Type::Map));

		REQUIRE(2 == source->size());

		StorageOf<Type::Str> name;
		REQUIRE((source->next("name")->get(name) && ("hypnos" == name)));

		StorageOf<Type::Int> age;
		REQUIRE((source->next("age")->get(age) && (42 == age)));
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

	SECTION("nested") {
		auto const profile =
		    node.is<Type::Map>().to<Profile>()
		        | (field("nested", &Profile::nested)
		           | field("value", &Nested::value))
		    && get;

		REQUIRE(node.ok());
		REQUIRE(21 == profile.nested.value);
	}

	SECTION("of MonoMapProp") {
		using H = Holder<std::unordered_map<std::string, int>>;

		Node node(Source::make({
		    _{"value", {_{"a", 1}, _{"b", 2}, _{"c", 3}}},
		}));

		auto const actual =
		    node.is<Type::Map>().to<H>()
		        | field("value", &H::value)
		    && get;

		std::unordered_map<std::string, int> const expected{
		    {"a", 1},
		    {"b", 2},
		    {"c", 3},
		};

		REQUIRE(expected == actual.value);
	}

	SECTION("of MonoListProp") {
		using H = Holder<std::vector<int>>;

		Node node(Source::make({
		    _{"value", {1, 2, 3}},
		}));

		auto const actual =
		    node.is<Type::Map>().to<H>()
		        | field("value", &H::value)
		    && get;

		std::vector<int> const expected{1, 2, 3};

		REQUIRE(expected == actual.value);
	}
}

TEST_CASE("ListProp") {
	using namespace cray;

	Node node(Source::null());

	node.is<Type::List>().of(prop<Type::Int>().mutipleOf(42)).size(2 < x);
}

TEST_CASE("MonoListProp") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	SECTION("::defaultValue") {
		Node null_node(Source::null());
		null_node.is<Type::List>().of<Type::Int>().get();
		REQUIRE(!null_node.ok());

		auto const value = null_node.is<Type::List>().of<Type::Int>().withDefault({7, 42, 53});
		REQUIRE(null_node.ok());
		REQUIRE(3 == value.size());
		REQUIRE(42 == value.at(1));
	}

	SECTION("::encodeDefaultValueInto") {
		auto source = Source::make(nullptr);
		REQUIRE(!source->is(Type::List));

		auto p = detail::getProp(prop<Type::List>().of<Type::Int>().defaultValue({2, 4, 8}));
		p->encodeDefaultValueInto(*source);
		REQUIRE(source->is(Type::List));

		REQUIRE(3 == source->size());

		StorageOf<Type::Int> v;
		REQUIRE((source->next(0)->get(v) && (2 == v)));
		REQUIRE((source->next(1)->get(v) && (4 == v)));
		REQUIRE((source->next(2)->get(v) && (8 == v)));
	}

	SECTION("::size") {
		Node node(Source::make({3, 5, 13, 21, 42}));

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
		Node node(Source::make({3, 5, 13, 21, 42}));

		auto const value = node.is<Type::List>().of<Type::Int>().get();
		REQUIRE(node.ok());
		REQUIRE(5 == value.size());
		REQUIRE(13 == value.at(2));
	}

	SECTION("nested") {
		SECTION("::get") {
			Node node(Source::make({
			    {1, 2, 3},
			    {4, 5, 6},
			    {7, 8, 9},
			}));

			std::vector<std::vector<StorageOf<Type::Int>>> const expected{
			    {1, 2, 3},
			    {4, 5, 6},
			    {7, 8, 9},
			};

			auto const actual = node.is<Type::List>().of(prop<Type::List>().of<Type::Int>()).get();
			REQUIRE(expected == actual);
		}
	}

	SECTION("of MonoMap") {
		SECTION("::get") {
			Node node(Source::make({
			    {_{"a", 0}, _{"b", 1}, _{"c", 2}},
			    {_{"d", 3}, _{"e", 4}, _{"f", 5}},
			    {_{"g", 6}, _{"h", 7}, _{"i", 8}},
			}));

			std::vector<std::unordered_map<std::string, StorageOf<Type::Int>>> const expected{
			    {{"a", 0}, {"b", 1}, {"c", 2}},
			    {{"d", 3}, {"e", 4}, {"f", 5}},
			    {{"g", 6}, {"h", 7}, {"i", 8}},
			};

			auto const actual = node.is<Type::List>().of(prop<Type::Map>().of<Type::Int>()).get();
			REQUIRE(expected == actual);
		}
	}
}

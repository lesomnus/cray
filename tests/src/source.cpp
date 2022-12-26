#include <algorithm>
#include <memory>
#include <utility>

#include <catch2/catch_template_test_macros.hpp>

#include <cray/source.hpp>
#include <cray/types.hpp>

template<typename T>
bool eq(std::shared_ptr<cray::Source> src, T storage) {
	assert(src != nullptr);

	auto const answer = storage;
	return (src->get(storage)) && (answer == storage);
}

TEST_CASE("Source::Entry") {
	using namespace cray;
	using Entry = Source::Entry;

	StorageOf<Type::Nil>  value_nil;
	StorageOf<Type::Bool> value_true  = false;
	StorageOf<Type::Bool> value_false = true;
	StorageOf<Type::Int>  value_int;
	StorageOf<Type::Num>  value_num;
	StorageOf<Type::Str>  value_str;

	SECTION("Nil") {
		auto const entry = Entry(nullptr);
		REQUIRE(entry.source->is(Type::Nil));
		REQUIRE(!entry.source->is(Type::Bool));
		REQUIRE(!entry.source->is(Type::Int));
		REQUIRE(!entry.source->is(Type::Num));
		REQUIRE(!entry.source->is(Type::Str));
		REQUIRE(!entry.source->is(Type::Map));
		REQUIRE(!entry.source->is(Type::List));

		REQUIRE(entry.source->get(value_nil));
		REQUIRE(!entry.source->get(value_true));
		REQUIRE(!entry.source->get(value_false));
		REQUIRE(!entry.source->get(value_int));
		REQUIRE(!entry.source->get(value_num));
		REQUIRE(!entry.source->get(value_str));
	}

	SECTION("Bool") {
		auto const entry_true  = Entry(true);
		auto const entry_false = Entry(false);
		REQUIRE(!entry_true.source->is(Type::Nil));
		REQUIRE(entry_true.source->is(Type::Bool));
		REQUIRE(!entry_true.source->is(Type::Int));
		REQUIRE(!entry_true.source->is(Type::Num));
		REQUIRE(!entry_true.source->is(Type::Str));
		REQUIRE(!entry_true.source->is(Type::Map));
		REQUIRE(!entry_true.source->is(Type::List));

		REQUIRE(!entry_true.source->get(value_nil));
		REQUIRE(entry_true.source->get(value_true));
		REQUIRE(!entry_true.source->get(value_int));
		REQUIRE(!entry_true.source->get(value_num));
		REQUIRE(!entry_true.source->get(value_str));

		REQUIRE(!entry_false.source->get(value_nil));
		REQUIRE(entry_false.source->get(value_false));
		REQUIRE(!entry_false.source->get(value_int));
		REQUIRE(!entry_false.source->get(value_num));
		REQUIRE(!entry_false.source->get(value_str));

		REQUIRE(true == value_true);
		REQUIRE(false == value_false);
	}

	SECTION("Int") {
		auto const entry = Entry(42);
		REQUIRE(!entry.source->is(Type::Nil));
		REQUIRE(!entry.source->is(Type::Bool));
		REQUIRE(entry.source->is(Type::Int));
		REQUIRE(!entry.source->is(Type::Num));
		REQUIRE(!entry.source->is(Type::Str));
		REQUIRE(!entry.source->is(Type::Map));
		REQUIRE(!entry.source->is(Type::List));

		REQUIRE(!entry.source->get(value_nil));
		REQUIRE(!entry.source->get(value_true));
		REQUIRE(!entry.source->get(value_false));
		REQUIRE(entry.source->get(value_int));
		REQUIRE(!entry.source->get(value_num));
		REQUIRE(!entry.source->get(value_str));
		REQUIRE(42 == value_int);
	}

	SECTION("Num") {
		auto const entry = Entry(3.14);
		REQUIRE(!entry.source->is(Type::Nil));
		REQUIRE(!entry.source->is(Type::Bool));
		REQUIRE(!entry.source->is(Type::Int));
		REQUIRE(entry.source->is(Type::Num));
		REQUIRE(!entry.source->is(Type::Str));
		REQUIRE(!entry.source->is(Type::Map));
		REQUIRE(!entry.source->is(Type::List));

		REQUIRE(!entry.source->get(value_nil));
		REQUIRE(!entry.source->get(value_true));
		REQUIRE(!entry.source->get(value_false));
		REQUIRE(!entry.source->get(value_int));
		REQUIRE(entry.source->get(value_num));
		REQUIRE(!entry.source->get(value_str));
		REQUIRE(3.14 == value_num);
	}

	SECTION("Str") {
		auto const entry = Entry("hypnos");
		REQUIRE(!entry.source->is(Type::Nil));
		REQUIRE(!entry.source->is(Type::Bool));
		REQUIRE(!entry.source->is(Type::Int));
		REQUIRE(!entry.source->is(Type::Num));
		REQUIRE(entry.source->is(Type::Str));
		REQUIRE(!entry.source->is(Type::Map));
		REQUIRE(!entry.source->is(Type::List));

		REQUIRE(!entry.source->get(value_nil));
		REQUIRE(!entry.source->get(value_true));
		REQUIRE(!entry.source->get(value_false));
		REQUIRE(!entry.source->get(value_int));
		REQUIRE(!entry.source->get(value_num));
		REQUIRE(entry.source->get(value_str));
		REQUIRE("hypnos" == value_str);
	}

	SECTION("Map") {
		using _ = Entry::MapValueType;

		auto const entry = Entry({
		    _{"a", nullptr},
		    _{"b", true},
		    _{"c", false},
		    _{"d", 42},
		    _{"e", 3.14},
		    _{"f", "hypnos"},
		});
		REQUIRE(!entry.source->is(Type::Nil));
		REQUIRE(!entry.source->is(Type::Bool));
		REQUIRE(!entry.source->is(Type::Int));
		REQUIRE(!entry.source->is(Type::Num));
		REQUIRE(!entry.source->is(Type::Str));
		REQUIRE(entry.source->is(Type::Map));
		REQUIRE(!entry.source->is(Type::List));

		REQUIRE(6 == entry.source->size());

		std::vector<std::string> const expected_keys{"a", "b", "c", "d", "e", "f"};
		entry.source->keys([&expected_keys](std::size_t index, std::string const& key) {
			REQUIRE(expected_keys[index] == key);
			return true;
		});

		REQUIRE(eq(entry.source->next("a"), StorageOf<Type::Nil>(nullptr)));
		REQUIRE(eq(entry.source->next("b"), StorageOf<Type::Bool>(true)));
		REQUIRE(eq(entry.source->next("c"), StorageOf<Type::Bool>(false)));
		REQUIRE(eq(entry.source->next("d"), StorageOf<Type::Int>(42)));
		REQUIRE(eq(entry.source->next("e"), StorageOf<Type::Num>(3.14)));
		REQUIRE(eq(entry.source->next("f"), StorageOf<Type::Str>("hypnos")));
	}

	SECTION("List") {
		auto const entry = Entry({
		    nullptr,
		    true,
		    false,
		    42,
		    3.14,
		    "hypnos",
		});
		REQUIRE(!entry.source->is(Type::Nil));
		REQUIRE(!entry.source->is(Type::Bool));
		REQUIRE(!entry.source->is(Type::Int));
		REQUIRE(!entry.source->is(Type::Num));
		REQUIRE(!entry.source->is(Type::Str));
		REQUIRE(!entry.source->is(Type::Map));
		REQUIRE(entry.source->is(Type::List));

		REQUIRE(6 == entry.source->size());
		REQUIRE(eq(entry.source->next(0), StorageOf<Type::Nil>(nullptr)));
		REQUIRE(eq(entry.source->next(1), StorageOf<Type::Bool>(true)));
		REQUIRE(eq(entry.source->next(2), StorageOf<Type::Bool>(false)));
		REQUIRE(eq(entry.source->next(3), StorageOf<Type::Int>(42)));
		REQUIRE(eq(entry.source->next(4), StorageOf<Type::Num>(3.14)));
		REQUIRE(eq(entry.source->next(5), StorageOf<Type::Str>("hypnos")));
	}

	SECTION("Composite") {
		using _ = Entry::MapValueType;

		auto const entry = Entry({
		    _{"arr", Entry({"foo", true, 21})},
		    _{
		        "map",
		        Entry({
		            _{"null", nullptr},
		            _{"answer", 42},
		            _{"pi", 3.14},
		        })},
		    _{
		        "nested_arr",
		        Entry({
		            Entry({"0", 1, 2.0}),
		            Entry({3, "4", "5"}),
		            Entry({6.0, "7", 8}),
		        }),
		    },
		    _{
		        "nested_map",
		        Entry({
		            _{"age", 28},
		            _{"name", "lesomnus"},
		        }),
		    },
		});

		auto const arr = entry.source->next("arr");
		REQUIRE(eq(arr->next(0), StorageOf<Type::Str>("foo")));
		REQUIRE(eq(arr->next(1), StorageOf<Type::Bool>(true)));
		REQUIRE(eq(arr->next(2), StorageOf<Type::Int>(21)));

		auto const map = entry.source->next("map");
		REQUIRE(eq(map->next("null"), StorageOf<Type::Nil>()));
		REQUIRE(eq(map->next("answer"), StorageOf<Type::Int>(42)));
		REQUIRE(eq(map->next("pi"), StorageOf<Type::Num>(3.14)));

		auto const nested_arr = entry.source->next("nested_arr");
		REQUIRE(eq(nested_arr->next(0)->next(0), StorageOf<Type::Str>("0")));
		REQUIRE(eq(nested_arr->next(0)->next(1), StorageOf<Type::Int>(1)));
		REQUIRE(eq(nested_arr->next(0)->next(2), StorageOf<Type::Num>(2.0)));
		REQUIRE(eq(nested_arr->next(1)->next(0), StorageOf<Type::Int>(3)));
		REQUIRE(eq(nested_arr->next(1)->next(1), StorageOf<Type::Str>("4")));
		REQUIRE(eq(nested_arr->next(1)->next(2), StorageOf<Type::Str>("5")));
		REQUIRE(eq(nested_arr->next(2)->next(0), StorageOf<Type::Num>(6.0)));
		REQUIRE(eq(nested_arr->next(2)->next(1), StorageOf<Type::Str>("7")));
		REQUIRE(eq(nested_arr->next(2)->next(2), StorageOf<Type::Int>(8)));

		auto const nested_map = entry.source->next("nested_map");
		REQUIRE(eq(nested_map->next("age"), StorageOf<Type::Int>(28)));
		REQUIRE(eq(nested_map->next("name"), StorageOf<Type::Str>("lesomnus")));
	}
}

TEST_CASE("Source::null") {
	using namespace cray;

	REQUIRE(nullptr != Source::null());

	REQUIRE(Source::null() == Source::null());
	REQUIRE(Source::null() == Source::null()->next("name"));
	REQUIRE(Source::null() == Source::null()->next(42));

	Source::null()->keys([](std::size_t index, std::string const& key) {
		FAIL();
		return false;
	});

	REQUIRE(0 == Source::null()->size());

	REQUIRE(!Source::null()->is(Type::Nil));
	REQUIRE(!Source::null()->is(Type::Bool));
	REQUIRE(!Source::null()->is(Type::Int));
	REQUIRE(!Source::null()->is(Type::Num));
	REQUIRE(!Source::null()->is(Type::Str));
	REQUIRE(!Source::null()->is(Type::Map));
	REQUIRE(!Source::null()->is(Type::List));

	auto const immutability = []<typename T>(T value) -> bool {
		auto const origin = value;
		auto const null   = Source::null();

		// ::get should returns false and does not alter given value.
		if(null->get(value) || (origin != value)) {
			return false;
		}

		// ::set does not have a side-effect.
		if(null->set(T()), null->get(value) || (origin != value)) {
			return false;
		}

		return true;
	};

	REQUIRE(immutability(StorageOf<Type::Nil>()));
	REQUIRE(immutability(StorageOf<Type::Bool>(true)));
	REQUIRE(immutability(StorageOf<Type::Int>(42)));
	REQUIRE(immutability(StorageOf<Type::Num>(3.14)));
	REQUIRE(immutability(StorageOf<Type::Str>("lesomnus")));
}

TEST_CASE("Source::make") {
	using namespace cray;
	using _ = Source::Entry::MapValueType;

	{
		auto const source = Source::make({true, false, 2.718, "3.14"});
		REQUIRE(eq(source->next(0), StorageOf<Type::Bool>(true)));
		REQUIRE(eq(source->next(1), StorageOf<Type::Bool>(false)));
		REQUIRE(eq(source->next(2), StorageOf<Type::Num>(2.718)));
		REQUIRE(eq(source->next(3), StorageOf<Type::Str>("3.14")));

		REQUIRE(nullptr == std::as_const(*source).next("name"));
		REQUIRE(nullptr == std::as_const(*source).next(42));
		REQUIRE(nullptr != std::as_const(*source).next(3));
		REQUIRE(nullptr == std::as_const(*source).next("name"));
		REQUIRE(nullptr == std::as_const(*source).next(42));
	}

	{
		auto const source = Source::make({
		    _{"a", true},
		    _{"b", false},
		    _{"c", 2.718},
		    _{"d", "3.14"},
		});
		REQUIRE(eq(source->next("a"), StorageOf<Type::Bool>(true)));
		REQUIRE(eq(source->next("b"), StorageOf<Type::Bool>(false)));
		REQUIRE(eq(source->next("c"), StorageOf<Type::Num>(2.718)));
		REQUIRE(eq(source->next("d"), StorageOf<Type::Str>("3.14")));

		REQUIRE(nullptr == std::as_const(*source).next("name"));
		REQUIRE(nullptr == std::as_const(*source).next(42));
		REQUIRE(nullptr != std::as_const(*source).next("d"));
		REQUIRE(nullptr == std::as_const(*source).next("name"));
		REQUIRE(nullptr == std::as_const(*source).next(42));
	}
}

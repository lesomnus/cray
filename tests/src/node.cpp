#include <memory>
#include <optional>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

#include <cray/node.hpp>
#include <cray/props.hpp>

TEST_CASE("is") {
	using namespace cray;
	using namespace cray::detail;

	Node node(Source::null());
	STATIC_REQUIRE(std::is_same_v<DescriberOf<PropOf<Type::Int>, GettableContext>, decltype(node.is<Type::Int>())>);
}

TEST_CASE("as") {
	using namespace cray;
	using _ = cray::Source::Entry::MapValueType;

	SECTION("optional") {
		{
			Node node(Source::null());

			node.as<int>();
			REQUIRE(!node.ok());
		}

		{
			Node node(Source::null());

			node.as<std::optional<int>>();
			REQUIRE(node.ok());
		}
	}

	auto const be = []<typename T>(Node node, T expected) -> bool {
		return (expected == node.as<T>()) && (expected == node.as<std::optional<T>>().value());
	};

	SECTION("nullptr") {
		REQUIRE(be(Node(Source::make(nullptr)), nullptr));
	}

	SECTION("bool") {
		REQUIRE(be(Node(Source::make(true)), true));
		REQUIRE(be(Node(Source::make(false)), false));
	}

	SECTION("integral") {
		Node node(Source::make(42));

		REQUIRE(be(node, (short)(42)));
		REQUIRE(be(node, (int)(42)));
		REQUIRE(be(node, (long)(42)));
		REQUIRE(be(node, (long long)(42)));

		REQUIRE(be(node, (unsigned short)(42)));
		REQUIRE(be(node, (unsigned int)(42)));
		REQUIRE(be(node, (unsigned long)(42)));
		REQUIRE(be(node, (unsigned long long)(42)));
	}

	SECTION("floating point") {
		Node node(Source::make(3.14));

		REQUIRE(be(node, (float)(3.14)));
		REQUIRE(be(node, (double)(3.14)));
		REQUIRE(be(node, (long double)(3.14)));
	}

	SECTION("std::string") {
		REQUIRE(be(Node(Source::make("hypnos")), std::string("hypnos")));
	}

	SECTION("std::vector") {
		REQUIRE(be(Node(Source::make({1, 2, 3})), std::vector<int>({1, 2, 3})));
	}

	SECTION("std::unordered_map") {
		REQUIRE(be(Node(Source::make({_{"a", 1}, _{"b", 2}, _{"c", 3}})), std::unordered_map<std::string, int>({{"a", 1}, {"b", 2}, {"c", 3}})));
	}
}

TEST_CASE("getProp") {
	using namespace cray;
	using namespace cray::detail;

	Node node(Source::null());
	node.is<Type::Int>();

	auto prop = getProp(node);
	REQUIRE(Type::Int == prop->type());
}

TEST_CASE("garbage collection") {
	using namespace cray;
	using namespace cray::detail;

	auto node = std::make_shared<Node>(Source::null());
	node->is<Type::Int>();

	auto prop = std::weak_ptr(getProp(*node));
	auto root = std::weak_ptr(getProp(*node)->prev);

	node.reset();
	REQUIRE(prop.expired());
	REQUIRE(root.expired());
}

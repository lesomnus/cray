#include <memory>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

#include <cray/detail/props.hpp>
#include <cray/node.hpp>

TEST_CASE("is") {
	using namespace cray;
	using namespace cray::detail;

	Node node(Source::null());
	STATIC_REQUIRE(std::is_same_v<DescriberOf<PropOf<Type::Int>, GettableContext>, decltype(node.is<Type::Int>())>);
}

TEST_CASE("as") {
	using namespace cray;

	Node node(Source::make(42));
	REQUIRE(42 == node.as<int>());
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

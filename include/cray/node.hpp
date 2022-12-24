#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <utility>

#include "cray/detail/prop.hpp"
#include "cray/types.hpp"

namespace cray {
namespace detail {

template<typename T>
struct NodePropGetter;

}

class Node {
   public:
	Node() = default;

	Node(std::shared_ptr<Source> source)
	    : prev_(std::make_shared<detail::RootProp>(std::move(source))) { }

	bool ok() const {
		return this->curr_()->ok();
	}

	template<Type T>
	auto is(Annotation annotation) {
		using P = detail::PropOf<T>;
		using D = detail::DescriberOf<P, detail::GettableContext>;

		auto curr = std::dynamic_pointer_cast<P>(this->curr_());
		if(curr == nullptr) {
			curr = makeProp<P>(std::move(annotation), this->prev_, this->ref_);
		} else {
			curr->annotation = std::move(annotation);
		}

		return D(std::move(curr));
	}

	template<Type T>
	inline auto is() {
		return this->is<T>(Annotation{});
	}

   private:
	std::shared_ptr<detail::Prop> curr_() const {
		return this->prev_->at(this->ref_);
	}

	template<typename T>
	friend struct detail::NodePropGetter;

	std::shared_ptr<detail::Prop> prev_;
	Reference                     ref_;
};

namespace detail {

template<typename T>
struct NodePropGetter {
	auto get() const {
		return this->value.curr_();
	}

	T value;
};

auto getProp(Node const& node) {
	return NodePropGetter<Node const&>{node}.get();
}

auto getProp(Node&& node) {
	return NodePropGetter<Node>{std::move(node)}.get();
}

}  // namespace detail
}  // namespace cray

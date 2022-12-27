#pragma once

#include <cassert>
#include <concepts>
#include <memory>
#include <string>
#include <utility>

#include "cray/detail/prop.hpp"
#include "cray/detail/props/poly-map.hpp"
#include "cray/types.hpp"

namespace cray {

namespace detail {

template<typename T>
struct NodePropGetter;

struct RequiredKey {
	std::string value;
};

}  // namespace detail

class Node {
   public:
	Node() = default;

	Node(std::shared_ptr<Source> source)
	    : prev_(std::make_shared<detail::RootProp>(std::move(source))) { }

	inline bool ok() const {
		return this->curr_()->ok();
	}

	template<Type T>
	auto is(Annotation annotation) {
		using P = detail::PropOf<T>;
		using D = detail::DescriberOf<P, detail::GettableContext>;

		auto curr = this->resolve_<P>(std::move(annotation));
		return D(std::move(curr));
	}

	template<Type T>
	inline auto is() {
		return this->is<T>(Annotation{});
	}

	template<typename T>
	inline auto as(Annotation annotation) {
		auto curr = this->resolve_<detail::PropFor<T>>(std::move(annotation));
		if constexpr(detail::IsOptional<T>) {
			return curr->opt();
		} else {
			return curr->get();
		}
	}

	template<typename T>
	inline T as() {
		return this->as<T>(Annotation{});
	}

	inline Node operator[](std::string key) {
		auto curr = this->resolve_<detail::PolyMpaProp>();
		return Node(std::move(curr), Reference(std::move(key)));
	}

	inline Node operator[](detail::RequiredKey key) {
		auto curr = this->resolve_<detail::PolyMpaProp>();
		curr->required_keys.insert(key.value);
		return Node(std::move(curr), Reference(std::move(key.value)));
	}

   private:
	Node(std::shared_ptr<detail::Prop> prev, Reference ref)
	    : prev_(std::move(prev))
	    , ref_(std::move(ref)) { }

	inline std::shared_ptr<detail::Prop> curr_() const {
		return this->prev_->at(this->ref_);
	}

	template<std::derived_from<detail::Prop> P>
	inline std::shared_ptr<P> resolve_(Annotation annotation) const {
		auto curr = std::dynamic_pointer_cast<P>(this->curr_());
		if(curr == nullptr) {
			curr = detail::makeProp<P>(std::move(annotation), this->prev_, this->ref_);
		}

		return curr;
	}

	template<std::derived_from<detail::Prop> P>
	inline std::shared_ptr<P> resolve_() const {
		return this->resolve_<P>(Annotation{});
	}

	template<typename T>
	friend struct detail::NodePropGetter;

	std::shared_ptr<detail::Prop> prev_;
	Reference                     ref_;
};

inline constexpr detail::RequiredKey req(std::string key) noexcept {
	return detail::RequiredKey{.value = std::move(key)};
}

namespace detail {

template<typename T>
struct NodePropGetter {
	auto get() const {
		return this->value.curr_();
	}

	T value;
};

inline auto getProp(Node const& node) {
	return NodePropGetter<Node const&>{node}.get();
}

inline auto getProp(Node&& node) {
	return NodePropGetter<Node>{std::move(node)}.get();
}

}  // namespace detail
}  // namespace cray

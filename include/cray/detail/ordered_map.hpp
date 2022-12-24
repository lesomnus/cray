#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cray {
namespace detail {

// TODO: implement erase; iterator must be re-implemented.
// Erase may not be needed.

template<typename Key, typename T>
class OrderedMap {
   public:
	template<typename Storage>
	struct basic_iterator {
		using value_type = std::conditional_t<std::is_const_v<Storage>, std::pair<Key const, T> const, std::pair<Key const, T>>;

		value_type const& operator*() const {
			return *this->ptr->values_.find(this->ptr->order_.at(this->index));
		}

		value_type& operator*() {
			return *this->ptr->values_.find(this->ptr->order_.at(this->index));
		}

		value_type const* operator->() const {
			return this->ptr->values_.find(this->ptr->order_.at(this->index)).operator->();
		}

		value_type* operator->() {
			return this->ptr->values_.find(this->ptr->order_.at(this->index)).operator->();
		}

		basic_iterator operator++() {
			++this->index;
			return *this;
		}

		basic_iterator operator++(int) {
			basic_iterator rst = *this;
			++this->index;
			return rst;
		}

		bool operator==(basic_iterator const& other) const {
			return this->ptr == other.ptr && this->index == other.index;
		}

		Storage*    ptr;
		std::size_t index;
	};

	using value_type     = std::pair<Key const, T>;
	using iterator       = basic_iterator<OrderedMap>;
	using const_iterator = basic_iterator<OrderedMap const>;

	OrderedMap() = default;

	OrderedMap(std::initializer_list<value_type> values) {
		this->order_.reserve(values.size());
		for(auto& value: values) {
			this->insert_or_assign(value.first, value.second);
		}
	}

	std::pair<iterator, bool> insert(value_type const& value) {
		auto const [it, ok] = this->values_.insert(value);
		return emplace_key_(it->first, ok);
	}

	std::pair<iterator, bool> insert(value_type&& value) {
		auto const [it, ok] = this->values_.insert(std::move(value));
		return emplace_key_(it->first, ok);
	}

	template<typename M>
	std::pair<iterator, bool> insert_or_assign(Key const& key, M&& value) {
		auto const [it, ok] = this->values_.insert_or_assign(key, std::forward<M>(value));
		return emplace_key_(it->first, ok);
	}

	template<typename M>
	std::pair<iterator, bool> insert_or_assign(Key&& key, M&& value) {
		auto const [it, ok] = this->values_.insert_or_assign(std::move(key), std::forward<M>(value));
		return emplace_key_(it->first, ok);
	}

	std::size_t size() const {
		return this->order_.size();
	}

	T const& at(Key const& key) const {
		return this->values_.at(key);
	}

	T& at(Key const& key) {
		return this->values_.at(key);
	}

	const_iterator find(Key const& key) const {
		if(!this->values_.contains(key)) {
			return this->cend();
		}

		auto const it = std::find(this->order_.cbegin(), this->order_.cend(), key);
		assert(it != this->order_.cend());

		return const_iterator{this, static_cast<std::size_t>(it - this->order_.cbegin())};
	}

	iterator find(Key const& key) {
		auto const it = std::as_const(this)->find(key);
		return iterator{it.ptr, it.index};
	}

	const_iterator begin() const {
		return const_iterator{this, 0};
	}

	const_iterator end() const {
		return const_iterator{this, this->order_.size()};
	}

	iterator begin() {
		return iterator{this, 0};
	}

	iterator end() {
		return iterator{this, this->order_.size()};
	}

	const_iterator cbegin() const {
		return const_iterator{this, 0};
	}

	const_iterator cend() const {
		return const_iterator{this, this->order_.size()};
	}

	const_iterator cbegin() {
		return const_iterator{this, 0};
	}

	const_iterator cend() {
		return const_iterator{this, this->order_.size()};
	}

	T& operator[](Key const& key) {
		auto const [it, ok] = this->values_.insert(std::make_pair(key, T()));
		if(ok) {
			this->order_.emplace_back(it->first);
		}

		return it->second;
	}

	T& operator[](Key&& key) {
		auto const [it, ok] = this->values_.insert(std::make_pair(std::move(key), T()));
		if(ok) {
			this->order_.emplace_back(it->first);
		}

		return it->second;
	}

   private:
	std::pair<iterator, bool> emplace_key_(Key const& key, bool is_new) {
		auto index = this->order_.cend();
		if(is_new) {
			index = this->order_.emplace(this->order_.cend(), key);
		} else {
			index = std::find(this->order_.cbegin(), this->order_.cend(), key);
		}

		return std::make_pair(iterator{this, this->order_.size()}, is_new);
	}

	std::vector<Key>           order_;
	std::unordered_map<Key, T> values_;
};

}  // namespace detail
}  // namespace cray

#pragma once

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>

namespace cray {
namespace detail {

template<typename T>
class OrderedSet {
   public:
	using value_type     = T;
	using iterator       = std::vector<T>::iterator;
	using const_iterator = std::vector<T>::const_iterator;

	OrderedSet() = default;

	OrderedSet(std::initializer_list<value_type> values) {
		this->values_.reserve(values.size());
		for(auto& value: values) {
			this->insert(std::move(value));
		}
	}

	inline std::size_t size() const {
		return this->values_.size();
	}

	inline bool empty() const {
		return this->values_.empty();
	}

	inline const_iterator find(value_type const& value) const {
		return std::ranges::find(this->values_.begin(), this->values_.end(), value);
	}

	inline iterator find(value_type const& value) {
		return std::ranges::find(this->values_.begin(), this->values_.end(), value);
	}

	inline bool contains(value_type const& value) const {
		return this->find(value) != this->end();
	}

	std::pair<iterator, bool> insert(value_type const& value) {
		auto const it = this->find(value);
		if(it != this->values_.end()) {
			return std::make_pair(it, false);
		} else {
			return std::make_pair(this->values_.insert(this->values_.end(), value), true);
		}
	}

	std::pair<iterator, bool> insert(value_type&& value) {
		auto const it = this->find(value);
		if(it != this->values_.end()) {
			return std::make_pair(it, false);
		} else {
			return std::make_pair(this->values_.insert(this->values_.end(), std::move(value)), true);
		}
	}

	inline iterator erase(const_iterator first, const_iterator last) {
		return this->values_.erase(first, last);
	}

	inline const_iterator begin() const {
		return this->values_.begin();
	}

	inline const_iterator end() const {
		return this->values_.end();
	}

	inline iterator begin() {
		return this->values_.begin();
	}

	inline iterator end() {
		return this->values_.end();
	}

	inline const_iterator cbegin() const {
		return this->values_.cbegin();
	}

	inline const_iterator cend() const {
		return this->values_.cend();
	}

   private:
	std::vector<T> values_;
};

}  // namespace detail
}  // namespace cray

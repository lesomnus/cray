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

	std::size_t size() const {
		return this->values_.size();
	}

	const_iterator find(value_type const& value) const {
		return std::find(this->values_.begin(), this->values_.end(), value);
	}

	iterator find(value_type const& value) {
		return std::find(this->values_.begin(), this->values_.end(), value);
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

	iterator erase(const_iterator first, const_iterator last) {
		return this->values_.erase(first, last);
	}

	const_iterator begin() const {
		return this->values_.begin();
	}

	const_iterator end() const {
		return this->values_.end();
	}

	iterator begin() {
		return this->values_.begin();
	}

	iterator end() {
		return this->values_.end();
	}

	const_iterator cbegin() const {
		return this->values_.cbegin();
	}

	const_iterator cend() const {
		return this->values_.cend();
	}

	const_iterator cbegin() {
		return this->values_.cbegin();
	}

	const_iterator cend() {
		return this->values_.cend();
	}

   private:
	std::vector<T> values_;
};

}  // namespace detail
}  // namespace cray

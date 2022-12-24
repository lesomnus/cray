#include <cassert>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "cray/detail/ordered_map.hpp"
#include "cray/source.hpp"
#include "cray/types.hpp"

namespace cray {

namespace {

using ScalarStorage = std::variant<
    StorageOf<Type::Nil>,
    StorageOf<Type::Bool>,
    StorageOf<Type::Int>,
    StorageOf<Type::Num>,
    StorageOf<Type::Str>>;

class Entry_: public Source {
   public:
	virtual void assign(Reference const& ref, std::shared_ptr<Entry_> entry) = 0;
};

class RootEntry: public Entry_ {
   public:
	RootEntry(std::shared_ptr<Entry_> value)
	    : value(std::move(value)) { }

	std::shared_ptr<Source> next(Reference&& ref) override {
		return this->value;
	}

	std::shared_ptr<Source> next(Reference const& ref) override {
		return this->value;
	}

	std::shared_ptr<Source> next(Reference const& ref) const override {
		return this->value;
	}

	std::vector<std::string> keys() const override {
		return {};
	}

	std::size_t size() const override {
		return 0;
	}

	bool has(Reference const& ref) const override {
		return value != nullptr;
	}

	bool is(Type type) const override {
		return false;
	}

	// clang-format off
	bool get(StorageOf<Type::Nil>   value) const override { return false; }
	bool get(StorageOf<Type::Bool>& value) const override { return false; }
	bool get(StorageOf<Type::Int>&  value) const override { return false; }
	bool get(StorageOf<Type::Num>&  value) const override { return false; }
	bool get(StorageOf<Type::Str>&  value) const override { return false; }

	void set(StorageOf<Type::Nil>        value) override { }
	void set(StorageOf<Type::Bool>       value) override { }
	void set(StorageOf<Type::Int>        value) override { }
	void set(StorageOf<Type::Num>        value) override { }
	void set(StorageOf<Type::Str> const& value) override { }
	void set(StorageOf<Type::Str>&&      value) override { }
	// clang-format on

	void assign(Reference const& ref, std::shared_ptr<Entry_> entry) override {
		value = std::move(entry);
	}

	std::shared_ptr<Source> value;
};

class ScalarEntry: public Entry_ {
   public:
	ScalarEntry() = default;

	ScalarEntry(ScalarStorage value)
	    : value(std::move(value)) { }

	std::shared_ptr<Source> next(Reference&& ref) override {
		throw detail::InvalidAccessError();
	}

	std::shared_ptr<Source> next(Reference const& ref) override {
		throw detail::InvalidAccessError();
	}

	std::shared_ptr<Source> next(Reference const& ref) const override {
		throw detail::InvalidAccessError();
	}

	std::vector<std::string> keys() const override {
		return {};
	}

	std::size_t size() const override {
		return 0;
	}

	bool has(Reference const& ref) const override {
		return false;
	}

	bool is(Type type) const override {
		switch(type) {
		case Type::Nil: return std::holds_alternative<StorageOf<Type::Nil>>(this->value);
		case Type::Bool: return std::holds_alternative<StorageOf<Type::Bool>>(this->value);
		case Type::Int: return std::holds_alternative<StorageOf<Type::Int>>(this->value);
		case Type::Num: return std::holds_alternative<StorageOf<Type::Num>>(this->value);
		case Type::Str: return std::holds_alternative<StorageOf<Type::Str>>(this->value);

		default: return false;
		}
	}

	// clang-format off
	bool get(StorageOf<Type::Nil>   value) const { return this->get_(value); }
	bool get(StorageOf<Type::Bool>& value) const { return this->get_(value); }
	bool get(StorageOf<Type::Int>&  value) const { return this->get_(value); }
	bool get(StorageOf<Type::Num>&  value) const { return this->get_(value); }
	bool get(StorageOf<Type::Str>&  value) const { return this->get_(value); }

	void set(StorageOf<Type::Nil>        value) { this->value = value; }
	void set(StorageOf<Type::Bool>       value) { this->value = value; }
	void set(StorageOf<Type::Int>        value) { this->value = value; }
	void set(StorageOf<Type::Num>        value) { this->value = value; }
	void set(StorageOf<Type::Str> const& value) { this->value = value; }
	void set(StorageOf<Type::Str>&&      value) { this->value = value; }
	// clang-format on

	void assign(Reference const& ref, std::shared_ptr<Entry_> entry) override {
		throw detail::InvalidAccessError();
	}

	ScalarStorage value;

   private:
	template<typename T>
	bool get_(T& value) const {
		auto const* v = std::get_if<T>(&this->value);
		if(v == nullptr) {
			return false;
		}

		value = *v;
		return true;
	}
};

class MapEntry: public Entry_ {
   public:
	MapEntry() = default;

	MapEntry(std::initializer_list<std::pair<std::string, Source::Entry>> values) {
		for(auto& value: values) {
			auto entry = std::dynamic_pointer_cast<Entry_>(std::move(value.second.source));
			assert(entry != nullptr);

			this->values.insert_or_assign(std::move(value.first), std::move(entry));
		}
	}

	std::shared_ptr<Source> next(Reference&& ref) override {
		return this->values[std::move(ref).key()];
	}

	std::shared_ptr<Source> next(Reference const& ref) override {
		return this->values[ref.key()];
	}

	std::shared_ptr<Source> next(Reference const& ref) const override {
		auto const it = this->values.find(ref.key());
		if(it == this->values.cend()) {
			return nullptr;
		}

		return this->values.at(ref.key());
	}

	std::vector<std::string> keys() const override {
		std::vector<std::string> rst;
		rst.reserve(this->values.size());
		for(auto const& [key, _]: this->values) {
			rst.emplace_back(key);
		}

		return rst;
	}

	std::size_t size() const override {
		return this->values.size();
	}

	bool has(Reference const& ref) const override {
		if(ref.isIndex()) {
			return false;
		}

		auto const it = this->values.find(ref.key());
		return it != this->values.cend();
	}

	bool is(Type type) const override {
		return type == Type::Map;
	}

	// clang-format off
	bool get(StorageOf<Type::Nil>   value) const override { return false; }
	bool get(StorageOf<Type::Bool>& value) const override { return false; }
	bool get(StorageOf<Type::Int>&  value) const override { return false; }
	bool get(StorageOf<Type::Num>&  value) const override { return false; }
	bool get(StorageOf<Type::Str>&  value) const override { return false; }

	void set(StorageOf<Type::Nil>        value) override { }
	void set(StorageOf<Type::Bool>       value) override { }
	void set(StorageOf<Type::Int>        value) override { }
	void set(StorageOf<Type::Num>        value) override { }
	void set(StorageOf<Type::Str> const& value) override { }
	void set(StorageOf<Type::Str>&&      value) override { }
	// clang-format on

	void assign(Reference const& ref, std::shared_ptr<Entry_> entry) override {
		this->values[ref.key()] = std::move(entry);
	}

	detail::OrderedMap<std::string, std::shared_ptr<Entry_>> values;
};

class ListEntry: public Entry_ {
   public:
	ListEntry() = default;

	ListEntry(std::initializer_list<Source::Entry> values) {
		this->values.reserve(values.size());
		for(auto& value: values) {
			auto entry = std::dynamic_pointer_cast<Entry_>(std::move(value.source));
			assert(entry != nullptr);

			this->values.emplace_back(std::move(entry));
		}
	}

	std::shared_ptr<Source> next(Reference&& ref) override {
		return this->next(ref);
	}

	std::shared_ptr<Source> next(Reference const& ref) override {
		auto const index = ref.index();

		if(index >= this->values.size()) {
			this->values.resize(index);
		}

		return this->values[index];
	}

	std::shared_ptr<Source> next(Reference const& ref) const override {
		auto const index = ref.index();
		if(index >= this->values.size()) {
			return nullptr;
		}

		return this->values.at(index);
	}

	std::vector<std::string> keys() const override {
		return {};
	}

	std::size_t size() const override {
		return this->values.size();
	}

	bool has(Reference const& ref) const override {
		if(!ref.isIndex()) {
			return false;
		}

		auto const index = ref.index();
		return this->values.size() > index;
	}

	bool is(Type type) const override {
		return type == Type::List;
	}

	// clang-format off
	bool get(StorageOf<Type::Nil>   value) const override { return false; }
	bool get(StorageOf<Type::Bool>& value) const override { return false; }
	bool get(StorageOf<Type::Int>&  value) const override { return false; }
	bool get(StorageOf<Type::Num>&  value) const override { return false; }
	bool get(StorageOf<Type::Str>&  value) const override { return false; }

	void set(StorageOf<Type::Nil>        value) override { }
	void set(StorageOf<Type::Bool>       value) override { }
	void set(StorageOf<Type::Int>        value) override { }
	void set(StorageOf<Type::Num>        value) override { }
	void set(StorageOf<Type::Str> const& value) override { }
	void set(StorageOf<Type::Str>&&      value) override { }
	// clang-format on

	void assign(Reference const& ref, std::shared_ptr<Entry_> entry) override {
		auto const index = ref.index();

		if(index >= this->values.size()) {
			this->values.resize(index);
		}

		this->values[index] = std::move(entry);
	}

	std::vector<std::shared_ptr<Entry_>> values;
};

class Accessor: public Source {
   public:
	Accessor(std::shared_ptr<Entry_> prev, Reference const& ref)
	    : prev(std::move(prev))
	    , ref(ref) { }

	Accessor(std::shared_ptr<Entry_> prev, Reference&& ref)
	    : prev(std::move(prev))
	    , ref(std::move(ref)) { }

	std::shared_ptr<Source> next(Reference&& ref) override {
		auto curr = ref.isIndex() ? this->resolve_<ListEntry>() : this->resolve_<MapEntry>();
		return std::make_shared<Accessor>(std::move(curr), std::move(ref));
	}

	std::shared_ptr<Source> next(Reference const& ref) override {
		auto curr = ref.isIndex() ? this->resolve_<ListEntry>() : this->resolve_<MapEntry>();
		return std::make_shared<Accessor>(std::move(curr), ref);
	}

	std::shared_ptr<Source> next(Reference const& ref) const override {
		auto curr = this->curr_();
		if(!curr->has(ref)) {
			return nullptr;
		}

		return std::make_shared<Accessor>(std::move(curr), ref);
	}

	std::vector<std::string> keys() const override {
		auto const curr = this->curr_();
		if(curr == nullptr) {
			return {};
		}

		return curr->keys();
	}

	std::size_t size() const override {
		auto const curr = this->curr_();
		if(curr == nullptr) {
			return 0;
		}

		return curr->size();
	}

	bool has(Reference const& ref) const override {
		auto const curr = this->curr_();
		if(curr == nullptr) {
			return false;
		}

		return curr->has(ref);
	}

	bool is(Type type) const override {
		auto const curr = this->curr_();
		if(curr == nullptr) {
			return false;
		}

		return curr->is(type);
	}

	// clang-format off
	bool get(StorageOf<Type::Nil>   value) const override { return this->get_(value); }
	bool get(StorageOf<Type::Bool>& value) const override { return this->get_(value); }
	bool get(StorageOf<Type::Int>&  value) const override { return this->get_(value); }
	bool get(StorageOf<Type::Num>&  value) const override { return this->get_(value); }
	bool get(StorageOf<Type::Str>&  value) const override { return this->get_(value); }

	void set(StorageOf<Type::Nil>        value) override { this->set_(value); }
	void set(StorageOf<Type::Bool>       value) override { this->set_(value); }
	void set(StorageOf<Type::Int>        value) override { this->set_(value); }
	void set(StorageOf<Type::Num>        value) override { this->set_(value); }
	void set(StorageOf<Type::Str> const& value) override { this->set_(value); }
	void set(StorageOf<Type::Str>&&      value) override { this->set_(std::move(value)); }
	// clang-format on

	std::shared_ptr<Entry_> prev;
	Reference const         ref;

   private:
	std::shared_ptr<Entry_> curr_() const {
		auto prev = std::as_const(*this->prev).next(this->ref);
		return std::dynamic_pointer_cast<Entry_>(prev);
	}

	template<std::derived_from<Entry_> T>
	std::shared_ptr<Entry_> resolve_() const {
		auto curr = this->curr_();
		if(dynamic_cast<T*>(curr.get()) == nullptr) {
			return nullptr;
		}

		return curr;
	}

	template<std::derived_from<Entry_> T>
	std::shared_ptr<Entry_> resolve_() {
		auto curr = this->curr_();
		if(dynamic_cast<T*>(curr.get()) == nullptr) {
			curr = std::make_shared<T>();
			this->prev->assign(this->ref, curr);
		}

		return curr;
	}

	template<typename V>
	bool get_(V& value) const {
		auto const curr = this->resolve_<ScalarEntry>();
		if(curr == nullptr) {
			return false;
		}
		return curr->get(value);
	}

	template<typename V>
	bool get_(V& value) {
		auto const curr = this->resolve_<ScalarEntry>();
		return curr->get(value);
	}

	template<typename V>
	void set_(V const& value) {
		auto const curr = this->resolve_<ScalarEntry>();
		return curr->set(value);
	}
};

}  // namespace

// clang-format off
Source::Entry::Entry(StorageOf<Type::Nil>  value) : source(std::make_shared<ScalarEntry>(value)) { }
Source::Entry::Entry(StorageOf<Type::Bool> value) : source(std::make_shared<ScalarEntry>(value)) { }
Source::Entry::Entry(StorageOf<Type::Int>  value) : source(std::make_shared<ScalarEntry>(value)) { }
Source::Entry::Entry(StorageOf<Type::Num>  value) : source(std::make_shared<ScalarEntry>(value)) { }
Source::Entry::Entry(StorageOf<Type::Str>  value) : source(std::make_shared<ScalarEntry>(value)) { }

Source::Entry::Entry(std::initializer_list<Entry>        values) : source(std::make_shared<ListEntry>(values)) { }
Source::Entry::Entry(std::initializer_list<MapValueType> values) : source(std::make_shared<MapEntry>(values)) { }

Source::Entry::Entry(int         value) : Entry(static_cast<StorageOf<Type::Int>>(value)) { }
Source::Entry::Entry(char const* value) : Entry(StorageOf<Type::Str>(value)) { }
// clang-format on

std::shared_ptr<Source> Source::make(Source::Entry value) {
	auto entry = std::dynamic_pointer_cast<Entry_>(std::move(value.source));
	assert(entry != nullptr);

	return std::make_shared<Accessor>(std::make_shared<RootEntry>(std::move(entry)), Reference());
}

}  // namespace cray

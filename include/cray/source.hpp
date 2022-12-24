#pragma once

#include <any>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <unordered_map>

#include "cray/types.hpp"

namespace cray {

class Source {
   public:
	struct Entry {
		using MapValueType = std::pair<std::string, Entry>;

		Entry(StorageOf<Type::Nil> value);
		Entry(StorageOf<Type::Bool> value);
		Entry(StorageOf<Type::Int> value);
		Entry(StorageOf<Type::Num> value);
		Entry(StorageOf<Type::Str> value);

		Entry(std::initializer_list<Entry> values);
		Entry(std::initializer_list<MapValueType> values);

		Entry(int value);
		Entry(char const* value);

		std::shared_ptr<Source> source;
	};

	static std::shared_ptr<Source> null();
	static std::shared_ptr<Source> make(Entry entry);

	virtual ~Source() { }

	virtual std::shared_ptr<Source> next(Reference&& ref)            = 0;
	virtual std::shared_ptr<Source> next(Reference const& ref)       = 0;
	virtual std::shared_ptr<Source> next(Reference const& ref) const = 0;

	virtual std::vector<std::string> keys() const = 0;
	virtual std::size_t              size() const = 0;

	virtual bool has(Reference const& ref) const = 0;

	virtual bool is(Type type) const = 0;

	virtual bool get(StorageOf<Type::Nil> value) const   = 0;
	virtual bool get(StorageOf<Type::Bool>& value) const = 0;
	virtual bool get(StorageOf<Type::Int>& value) const  = 0;
	virtual bool get(StorageOf<Type::Num>& value) const  = 0;
	virtual bool get(StorageOf<Type::Str>& value) const  = 0;

	virtual void set(StorageOf<Type::Nil> value)        = 0;
	virtual void set(StorageOf<Type::Bool> value)       = 0;
	virtual void set(StorageOf<Type::Int> value)        = 0;
	virtual void set(StorageOf<Type::Num> value)        = 0;
	virtual void set(StorageOf<Type::Str> const& value) = 0;
	virtual void set(StorageOf<Type::Str>&& value)      = 0;
};

}  // namespace cray

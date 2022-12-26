#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "cray/source.hpp"
#include "cray/types.hpp"

namespace cray {

namespace {

class NullSource: public Source {
   public:
	static std::shared_ptr<NullSource> const instance;

	std::shared_ptr<Source> next(Reference&& ref) override {
		return instance;
	};

	std::shared_ptr<Source> next(Reference const& ref) override {
		return instance;
	};

	std::shared_ptr<Source> next(Reference const& ref) const override {
		return instance;
	};

	void keys(std::function<bool(std::string const& key)> const& functor) const override {
	}

	std::size_t size() const override {
		return 0;
	};

	bool has(Reference const& ref) const override {
		return false;
	}

	bool is(Type type) const override {
		return false;
	}

	// clang-format off
	bool get(StorageOf<Type::Nil>   value) const override { return false; };
	bool get(StorageOf<Type::Bool>& value) const override { return false; };
	bool get(StorageOf<Type::Int>&  value) const override { return false; };
	bool get(StorageOf<Type::Num>&  value) const override { return false; };
	bool get(StorageOf<Type::Str>&  value) const override { return false; };

	void set(StorageOf<Type::Nil>        value) override {};
	void set(StorageOf<Type::Bool>       value) override {};
	void set(StorageOf<Type::Int>        value) override {};
	void set(StorageOf<Type::Num>        value) override {};
	void set(StorageOf<Type::Str> const& value) override {};
	void set(StorageOf<Type::Str>&&      value) override {};
	// clang-format on
};

const std::shared_ptr<NullSource> NullSource::instance = std::make_shared<NullSource>();

}  // namespace

std::shared_ptr<Source> Source::null() {
	return NullSource::instance;
}

}  // namespace cray

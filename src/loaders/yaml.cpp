#include "cray/load.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include <yaml-cpp/yaml.h>

#include "cray/source.hpp"
#include "cray/types.hpp"

namespace cray {
namespace load {

namespace {

class YamlSource: public Source {
   public:
	YamlSource(YAML::Node const& node)
	    : node(std::move(node)) { }

	std::shared_ptr<Source> next(Reference&& ref) override {
		return this->next(std::as_const(ref));
	}

	std::shared_ptr<Source> next(Reference const& ref) override {
		return ref.visit([this](auto const& value) {
			return std::make_shared<YamlSource>(this->node[value]);
		});
	}

	std::shared_ptr<Source> next(Reference const& ref) const override {
		auto next_node = this->has_(ref);
		if(!next_node.IsDefined()) {
			return nullptr;
		}

		return std::make_shared<YamlSource>(std::move(next_node));
	}

	void keys(std::function<bool(std::string const& key)> const& functor) const override {
		for(auto const& next_node: this->node) {
			auto key = next_node.first.as<std::string>();
			functor(key);
		}
	}

	std::size_t size() const override {
		if(!(this->node.IsMap() || this->node.IsScalar())) {
			return 0;
		}

		return this->node.size();
	}

	bool has(Reference const& ref) const override {
		return this->has_(ref).IsDefined();
	}

	bool is(Type t) const override {
		switch(t) {
		case Type::Nil: return this->node.IsNull();
		case Type::Bool: return this->has_<bool>();
		case Type::Int: return this->has_<int>();
		case Type::Num: return this->has_<double>();
		case Type::Str: return this->node.IsScalar();
		case Type::Map: return this->node.IsMap();
		case Type::List: return this->node.IsSequence();

		default: throw detail::InvalidTypeError(t);
		}
	}

	// clang-format off
	bool get(StorageOf<Type::Nil>   value) const override { return this->node.IsNull(); }
	bool get(StorageOf<Type::Bool>& value) const override { return this->get_(value); }
	bool get(StorageOf<Type::Int>&  value) const override { return this->get_(value); }
	bool get(StorageOf<Type::Num>&  value) const override { return this->get_(value); }
	bool get(StorageOf<Type::Str>&  value) const override { return this->get_(value); }

	void set(StorageOf<Type::Nil>        value) override { this->node = YAML::Null; }
	void set(StorageOf<Type::Bool>       value) override { this->node = value; }
	void set(StorageOf<Type::Int>        value) override { this->node = value; }
	void set(StorageOf<Type::Num>        value) override { this->node = value; }
	void set(StorageOf<Type::Str> const& value) override { this->node = value; }
	void set(StorageOf<Type::Str>&&      value) override { this->node = std::move(value); }
	// clang-format on

	YAML::Node node;

   private:
	template<typename V>
	bool get_(V& value) const {
		try {
			value = this->node.as<V>();
		} catch(...) {
			return false;
		}
		return true;
	}

	template<typename V>
	bool has_() const {
		V value;
		return this->get_(value);
	}

	YAML::Node has_(Reference const& ref) const {
		YAML::Node next_node;
		if(ref.isIndex() && this->node.IsSequence()) {
			auto const index = ref.index();
			if(this->node.size() > index) {
				next_node = node[index];
			}
		} else if(ref.isKey() && this->node.IsMap()) {
			auto const& key = ref.key();
			next_node       = this->node[key];
		}

		return next_node;
	}
};

class YamlLoader: public Loader {
   public:
	std::shared_ptr<Source> load(std::istream& in) override {
		YAML::Node node = YAML::Load(in);
		return std::make_shared<YamlSource>(std::move(node));
	}
};

class YamlLoaderFactory: public LoaderFactory {
   public:
	std::string name() const {
		return "yaml";
	}

	std::shared_ptr<Loader> make() const {
		return std::make_shared<YamlLoader>();
	}
};

void* const _ = ([] {
	auto factory = std::make_shared<YamlLoaderFactory>();
	load::add("yaml", factory);
	load::add("json", factory);
	return nullptr;
})();

}  // namespace

}  // namespace load
}  // namespace cray

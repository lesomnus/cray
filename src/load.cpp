#include "cray/load.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace cray {

namespace {

LoaderRegistry global_registry;

}

bool LoaderRegistry::has(std::string const& name) const {
	return this->factories_.contains(name);
}

void LoaderRegistry::add(std::shared_ptr<LoaderFactory> factory) {
	auto name = factory->name();
	this->factories_.insert_or_assign(std::move(name), std::move(factory));
}

void LoaderRegistry::add(std::string name, std::shared_ptr<LoaderFactory> factory) {
	this->factories_.insert_or_assign(std::move(name), std::move(factory));
}

std::shared_ptr<LoaderFactory> LoaderRegistry::get(std::string const& name) const {
	auto it = this->factories_.find(name);
	if(it == this->factories_.cend()) {
		return nullptr;
	} else {
		return it->second;
	}
}

namespace loader_registry {

bool has(std::string const& name) {
	return global_registry.has(name);
}

void add(std::shared_ptr<LoaderFactory> factory) {
	global_registry.add(std::move(factory));
}

void add(std::string name, std::shared_ptr<LoaderFactory> factory) {
	global_registry.add(std::move(name), std::move(factory));
}

std::shared_ptr<LoaderFactory> get(std::string const& name) {
	return global_registry.get(name);
}

}  // namespace loader_registry

}  // namespace cray

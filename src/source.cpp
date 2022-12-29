#include "cray/source.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "cray/load.hpp"

namespace cray {

std::shared_ptr<Source> Source::load(std::string const& name, std::istream& in) {
	auto factory = cray::loader_registry::get(name);
	if(factory == nullptr) {
		return nullptr;
	}

	auto loader = factory->make();
	return loader->load(in);
}

std::shared_ptr<Source> Source::load(std::string const& name, std::filesystem::path const& path) {
	std::ifstream f(path);
	return Source::load(name, f);
}

}  // namespace cray

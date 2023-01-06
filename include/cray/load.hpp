#pragma once

#include <filesystem>
#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>

#include "cray/source.hpp"

namespace cray {

class Loader {
   public:
	virtual std::shared_ptr<Source> load(std::istream& in) = 0;
};

class LoaderFactory {
   public:
	virtual std::string name() const = 0;

	virtual std::shared_ptr<Loader> make() const = 0;
};

class LoaderRegistry {
   public:
	bool has(std::string const& name) const;

	void add(std::shared_ptr<LoaderFactory> factory);

	void add(std::string name, std::shared_ptr<LoaderFactory> factory);

	std::shared_ptr<LoaderFactory> get(std::string const& name) const;

   private:
	std::unordered_map<std::string, std::shared_ptr<LoaderFactory>> factories_;
};

namespace loader_registry {

bool has(std::string const& name);

void add(std::shared_ptr<LoaderFactory> factory);

void add(std::string name, std::shared_ptr<LoaderFactory> factory);

std::shared_ptr<LoaderFactory> get(std::string const& name);

}  // namespace loader_registry

namespace load {

inline std::shared_ptr<Source> fromJson(std::istream& in) {
	return Source::load("json", in);
}

inline std::shared_ptr<Source> fromJson(std::filesystem::path const& path) {
	return Source::load("json", path);
}

inline std::shared_ptr<Source> fromYaml(std::istream& in) {
	return Source::load("yaml", in);
}

inline std::shared_ptr<Source> fromYaml(std::filesystem::path const& path) {
	return Source::load("yaml", path);
}

}  // namespace load

}  // namespace cray

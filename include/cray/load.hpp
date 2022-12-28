#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>

#include "cray/source.hpp"

namespace cray {
namespace load {

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
	void add(std::shared_ptr<LoaderFactory> factory);

	void add(std::string name, std::shared_ptr<LoaderFactory> factory);

	std::shared_ptr<LoaderFactory> get(std::string const& name) const;

   private:
	std::unordered_map<std::string, std::shared_ptr<LoaderFactory>> factories_;
};

void add(std::shared_ptr<LoaderFactory> factory);

void add(std::string name, std::shared_ptr<LoaderFactory> factory);

std::shared_ptr<LoaderFactory> get(std::string const& name);

}  // namespace load
}  // namespace cray

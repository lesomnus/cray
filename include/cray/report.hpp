#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>

#include "cray/detail/prop.hpp"
#include "cray/node.hpp"
#include "cray/types.hpp"

namespace cray {

class Reporter {
   public:
	void report(std::ostream& dst, Node const node) const {
		this->report_(dst, *detail::getProp(node));
	}

	bool        use_tabs;
	std::size_t tab_size;

   protected:
	virtual void report_(std::ostream& dst, detail::Prop const& prop) const = 0;
};

namespace report {

void asYaml(std::ostream& dst, Node node);

void asJsonSchema(std::ostream& dst, Node node);

}  // namespace report

}  // namespace cray

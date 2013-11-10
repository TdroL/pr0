#ifndef SG_ROOT_HPP
#define SG_ROOT_HPP

#include "node.hpp"
#include <list>

namespace sg
{

class Root
{
public:
	std::list<Node *> nodes{};

	virtual void render();

	virtual ~Root() {}
};

}

#endif
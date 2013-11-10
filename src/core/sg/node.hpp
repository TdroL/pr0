#ifndef SG_NODE_HPP
#define SG_NODE_HPP

#include "object.hpp"
#include <list>

namespace sg
{

class Node
{
public:
	std::list<Node *> nodes{};
	std::list<Object *> objects{};

	virtual void render();

	virtual ~Node() {}
};

}

#endif
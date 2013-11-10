#include "node.hpp"

namespace sg
{

void Node::render()
{
	for (auto &node : nodes)
	{
		node->render();
	}

	for (auto &object : objects)
	{
		object->render();
	}
}

}
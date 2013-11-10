#include "root.hpp"

namespace sg
{

void Root::render()
{
	for (auto &node : nodes)
	{
		node->render();
	}
}

}
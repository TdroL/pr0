#ifndef APP_COMP_MESH_HPP
#define APP_COMP_MESH_HPP

#include <core/ecs/component.hpp>

namespace comp
{

struct Mesh : public ecs::Component<Mesh>
{
	size_t id = 0;
};

} // comp

#endif
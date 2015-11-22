#pragma once

#include <core/ecs/component.hpp>

namespace comp
{

struct Mesh : public ecs::Component<Mesh>
{
	size_t id = 0;
};

} // comp
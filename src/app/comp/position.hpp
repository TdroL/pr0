#pragma once

#include <core/ecs/component.hpp>
#include <core/rn.hpp>

namespace comp
{

struct Position : public ecs::Component<Position>
{
	glm::vec3 position{0.f};
};

} // comp
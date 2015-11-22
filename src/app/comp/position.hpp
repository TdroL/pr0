#pragma once

#include <core/ecs/component.hpp>
#include <glm/glm.hpp>

namespace comp
{

struct Position : public ecs::Component<Position>
{
	glm::vec3 position{0.f};
};

} // comp
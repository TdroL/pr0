#pragma once

#include <core/ecs/component.hpp>
#include <core/rn.hpp>

namespace comp
{

struct Rotation : public ecs::Component<Rotation>
{
	glm::vec3 rotation{0.f};
};

} // comp
#pragma once

#include <core/ecs/component.hpp>
#include <core/rn.hpp>

namespace comp
{

struct Material : public ecs::Component<Material>
{
	glm::vec4 diffuse{0.f, 0.f, 0.f, 1.f};
	float roughness = 0.f;
};

} // comp
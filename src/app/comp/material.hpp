#pragma once

#include <core/ecs/component.hpp>
#include <glm/glm.hpp>

namespace comp
{

struct Material : public ecs::Component<Material>
{
	glm::vec4 diffuse{0.f, 0.f, 0.f, 1.f};
	float shininess = 0.f;
};

} // comp
#pragma once

#include <core/ecs/component.hpp>
#include <core/rn.hpp>

namespace comp
{

struct PointLight : public ecs::Component<PointLight>
{
	glm::vec4 color{0.f, 0.f, 0.f, 1.f};
	// glm::vec3 spotDirection{0.f, 0.f, -1.f};
	// GLfloat spotExponent = 0.f;
	// GLfloat spotCutoff = 180.f;
	float intensity = 1.f;
	float radius = 1.f;
	float cutoff = 3.f;
	bool shadowCaster = false;
};

} // comp
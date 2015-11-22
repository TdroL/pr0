#pragma once

#include <core/ecs/component.hpp>
#include <glm/glm.hpp>

namespace comp
{

struct Projection : public ecs::Component<Projection>
{
	float fovy = 45.f;
	float aspect = 16.f / 9.f;
	float zNear = 0.01f; // 1 cm
	float zFar = 200.f; // 256 m

	glm::mat4 matrix{1.f};
	glm::mat4 invMatrix{1.f};
};

} // comp

#ifndef APP_COMP_DIRECTIONALLIGHT_HPP
#define APP_COMP_DIRECTIONALLIGHT_HPP

// #include <pch.hpp>
#include <core/ecs/component.hpp>
// #include <core/rn.hpp>
#include <glm/glm.hpp>

namespace comp
{

struct DirectionalLight : public ecs::Component<DirectionalLight>
{
	glm::vec4 ambient{0.f, 0.f, 0.f, 1.f};
	glm::vec4 color{0.f, 0.f, 0.f, 1.f};
	glm::vec3 direction{0.f, 0.f, -1.f};
	float intensity = 1.f;
	bool shadowCaster = false;
};

} // comp

#endif
#ifndef APP_COMP_POINTLIGHT_HPP
#define APP_COMP_POINTLIGHT_HPP

// #include <pch.hpp>
#include <core/ecs/component.hpp>
#include <glm/glm.hpp>

namespace comp
{

struct PointLight : public ecs::Component<PointLight>
{
	glm::vec4 color{0.f, 0.f, 0.f, 1.f};
	// glm::vec3 spotDirection{0.f, 0.f, -1.f};
	// GLfloat spotExponent = 0.f;
	// GLfloat spotCutoff = 180.f;
	// GLfloat constantAttenuation = 1.f;
	float intensity = 1.f;
	float linearAttenuation = 0.f;
	float quadraticAttenuation = 0.f;
	bool shadowCaster = false;
};

} // comp

#endif
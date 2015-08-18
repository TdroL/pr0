#ifndef APP_COMP_MATERIAL_HPP
#define APP_COMP_MATERIAL_HPP

// #include <pch.hpp>
#include <core/ecs/component.hpp>
// #include <core/rn.hpp>
#include <glm/glm.hpp>

namespace comp
{

struct Material : public ecs::Component<Material>
{
	glm::vec4 diffuse{0.f, 0.f, 0.f, 1.f};
	float shininess = 0.f;
};

} // comp

#endif
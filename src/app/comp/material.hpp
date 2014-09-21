#ifndef APP_COMP_MATERIAL_HPP
#define APP_COMP_MATERIAL_HPP

#include <core/ecs/component.hpp>
#include <glm/glm.hpp>
#include <core/rn.hpp>

namespace comp
{

struct Material : public ecs::Component<Material>
{
	glm::vec4 diffuse{0.f, 0.f, 0.f, 1.f};
	GLfloat shininess = 0.f;
};

} // comp

#endif
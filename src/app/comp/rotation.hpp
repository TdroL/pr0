#ifndef APP_COMP_ROTATION_HPP
#define APP_COMP_ROTATION_HPP

// #include <pch.hpp>
#include <core/ecs/component.hpp>
#include <glm/glm.hpp>

namespace comp
{

struct Rotation : public ecs::Component<Rotation>
{
	glm::vec3 rotation{0.f};
};

} // comp

#endif
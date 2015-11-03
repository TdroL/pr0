#ifndef APP_COMP_TRANSFORM_HPP
#define APP_COMP_TRANSFORM_HPP

// #include <pch.hpp>
#include <core/ecs/component.hpp>
#include <core/phs/sphere.hpp>
#include <core/phs/aabb.hpp>
#include <glm/glm.hpp>

namespace comp
{

struct Transform : public ecs::Component<Transform>
{
	glm::vec3 translation{0.0};
	glm::vec3 rotation{0.0};
	glm::vec3 scale{1.0};
};

} // comp

#endif
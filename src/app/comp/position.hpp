#ifndef APP_COMP_POSITION_HPP
#define APP_COMP_POSITION_HPP

// #include <pch.hpp>
#include <core/ecs/component.hpp>
#include <glm/glm.hpp>

namespace comp
{

struct Position : public ecs::Component<Position>
{
	glm::vec3 position{0.f};
};

} // comp

#endif
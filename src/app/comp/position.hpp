#ifndef APP_COMP_POSITION_HPP
#define APP_COMP_POSITION_HPP

#include <core/ecs/component.hpp>

namespace comp
{

struct Position : public ecs::Component<Position>
{
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

} // comp

#endif
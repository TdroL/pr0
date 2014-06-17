#ifndef APP_COMP_DIRECTION_HPP
#define APP_COMP_DIRECTION_HPP

#include <core/ecs/component.hpp>

namespace comp
{

struct Direction : public ecs::Component<Direction>
{
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

} // comp

#endif
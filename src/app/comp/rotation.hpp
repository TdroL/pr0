#ifndef APP_COMP_ROTATION_HPP
#define APP_COMP_ROTATION_HPP

#include <core/ecs/component.hpp>

namespace comp
{

struct Rotation : public ecs::Component<Rotation>
{
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

} // comp

#endif
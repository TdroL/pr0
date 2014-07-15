#ifndef APP_COMP_STENCIL_HPP
#define APP_COMP_STENCIL_HPP

#include <core/ecs/component.hpp>
#include <core/gl.hpp>

namespace comp
{

struct Stencil : public ecs::Component<Stencil>
{
	GLint ref = 0;
};

} // comp

#endif
#ifndef APP_COMP_STENCIL_HPP
#define APP_COMP_STENCIL_HPP

#include <core/ecs/component.hpp>
#include <core/rn.hpp>

namespace comp
{

struct Stencil : public ecs::Component<Stencil>
{
	enum {
		MASK_SHADED = 1,
		MASK_FLAT = 2,
		MASK_ALL = 1 | 2
	};

	GLint ref = 0;
};

} // comp

#endif
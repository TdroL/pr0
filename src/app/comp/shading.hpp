#pragma once

#include <core/ecs/component.hpp>

namespace comp
{

struct Shading : public ecs::Component<Shading>
{
	enum Group {
		GROUP_NONE,
		GROUP_SHADED,
		GROUP_FLAT
	};

	Group group = GROUP_NONE;
};

} // comp
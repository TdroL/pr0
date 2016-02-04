#pragma once

#include <core/ecs/component.hpp>
#include <core/phs/sphere.hpp>
#include <core/phs/aabb.hpp>

namespace comp
{

struct BoundingVolume : public ecs::Component<BoundingVolume>
{
	bool dirty = true;
	phs::Sphere sphere{};
};

} // comp

#ifndef APP_COMP_BOUNDINGOBJECT_HPP
#define APP_COMP_BOUNDINGOBJECT_HPP

#include <core/ecs/component.hpp>
#include <core/phs/sphere.hpp>
#include <core/phs/aabb.hpp>

namespace comp
{

struct BoundingObject : public ecs::Component<BoundingObject>
{
	bool dirty = true;
	phs::Sphere sphere{};
	phs::AABB aabb{};
};

} // comp

#endif
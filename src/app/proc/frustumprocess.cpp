#include "frustumprocess.hpp"

#include <app/comp/boundingobject.hpp>

namespace proc
{

using namespace comp;

bool FrustumProcess::isVisible(const ecs::Entity &entity, const phs::Frustum &frustum)
{
	return frustum.test(ecs::get<BoundingObject>(entity).sphere);
}

} // proc
#include <pch.hpp>

#include "frustumprocess.hpp"

#include <app/comp/boundingvolume.hpp>

namespace proc
{

using namespace comp;

bool FrustumProcess::isVisible(const ecs::Entity &entity, const phs::Frustum &frustum)
{
	return frustum.test(ecs::get<BoundingVolume>(entity).sphere);
}

} // proc
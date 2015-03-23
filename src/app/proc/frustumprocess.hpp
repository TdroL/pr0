#ifndef APP_PROC_FRUSTUMPROCESS_HPP
#define APP_PROC_FRUSTUMPROCESS_HPP

#include <core/ecs/entity.hpp>
#include <core/phs/frustum.hpp>

namespace proc
{

class FrustumProcess
{
public:
	static bool isVisible(const ecs::Entity &entity, const phs::Frustum &frustum);
};

} // proc

#endif
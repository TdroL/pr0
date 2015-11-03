#ifndef APP_PROC_REBUILDBOUNDINGVOLUMEPROCESS_HPP
#define APP_PROC_REBUILDBOUNDINGVOLUMEPROCESS_HPP

#include <core/ecs/entity.hpp>

namespace proc
{

class RebuildBoundingVolumeProcess
{
public:
	static void update(const ecs::Entity &entity);
};

} // proc

#endif
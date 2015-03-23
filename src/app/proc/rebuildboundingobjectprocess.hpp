#ifndef APP_PROC_REBUILDBOUNDINGOBJECTPROCESS_HPP
#define APP_PROC_REBUILDBOUNDINGOBJECTPROCESS_HPP

#include <core/ecs/entity.hpp>

namespace proc
{

class RebuildBoundingObjectProcess
{
public:
	static void update(const ecs::Entity &entity);
};

} // proc

#endif
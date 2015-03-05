#ifndef APP_PROC_TRANSFORMPROCESS_HPP
#define APP_PROC_TRANSFORMPROCESS_HPP

#include <core/ecs/entity.hpp>

namespace proc
{

class TransformProcess
{
public:
	static void update(const ecs::Entity &entity);
};

} // proc

#endif
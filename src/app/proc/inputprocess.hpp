#ifndef APP_PROC_INPUTPROCESS_HPP
#define APP_PROC_INPUTPROCESS_HPP

#include <core/ecs/entity.hpp>

namespace proc
{

class InputProcess
{
public:
	static void update(const ecs::Entity &entity);

	static void commandNoop(const ecs::Entity &entity);
	static void commandMoveForward(const ecs::Entity &entity);
	static void commandMoveBackward(const ecs::Entity &entity);
	static void commandStrafeLeft(const ecs::Entity &entity);
	static void commandStrafeRight(const ecs::Entity &entity);
	static void commandMoveUpward(const ecs::Entity &entity);
	static void commandMoveDownward(const ecs::Entity &entity);
};

} // app

#endif
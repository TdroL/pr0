#include "inputprocess.hpp"

#include <core/ngn/key.hpp>

#include <app/comp/input.hpp>
#include <app/comp/transform.hpp>
#include <app/comp/temporaltransform.hpp>

#include <iostream>

namespace proc
{

using namespace comp;
using namespace std;

namespace key = ngn::key;

void InputProcess::update(const ecs::Entity &entity)
{
	auto &input = ecs::get<Input>(entity);

	for (auto &keyCommand : input.keyCommands)
	{
		if ( ! (
			(
				keyCommand.onKeyState.test(Input::KEYSTATE_PRESS)
				&&
				key::hit(keyCommand.key)
			)
			||
			(
				keyCommand.onKeyState.test(Input::KEYSTATE_HOLD)
				&&
				key::pressed(keyCommand.key)
			)
			||
			(
				keyCommand.onKeyState.test(Input::KEYSTATE_RELEASE)
				&&
				key::wasPressed(keyCommand.key) && ! key::pressed(keyCommand.key)
			)
		))
		{
			continue;
		}

		switch (keyCommand.command)
		{
			case Input::COMMAND_NOOP:
			{
				commandNoop(entity);
				break;
			}
			case Input::COMMAND_MOVEFORWARD:
			{
				commandMoveForward(entity);
				break;
			}
			case Input::COMMAND_MOVEBACKWARD:
			{
				commandMoveBackward(entity);
				break;
			}
			case Input::COMMAND_STRAFELEFT:
			{
				commandStrafeLeft(entity);
				break;
			}
			case Input::COMMAND_STRAFERIGHT:
			{
				commandStrafeRight(entity);
				break;
			}
			case Input::COMMAND_MOVEUPWARD:
			{
				commandMoveUpward(entity);
				break;
			}
			case Input::COMMAND_MOVEDOWNWARD:
			{
				commandMoveDownward(entity);
				break;
			}
		}
	}
}

void InputProcess::commandNoop(const ecs::Entity &)
{
	/*
	glm::vec3 translate{0.0};
	glm::vec3 rotate{0.0};

	translate.x = (key::pressed('d') - key::pressed('a'));
	translate.y = (key::pressed(KEY_SPACE) - key::pressed(KEY_CTRL));
	translate.z = (key::pressed('s') - key::pressed('w'));

	if (translate.x != 0.f || translate.z != 0.f)
	{
		glm::vec2 planar = glm::normalize(translate.xz());

		translate.x = planar.x;
		translate.z = planar.y;
	}

	rotate.y = (key::pressed(KEY_LEFT) - key::pressed(KEY_RIGHT));
	rotate.x = (key::pressed(KEY_UP) - key::pressed(KEY_DOWN));

	translate *= 20.0 * ngn::dt;
	rotate *= 90.0 * ngn::dt;

	proc::Camera::update(cameraId, translate, rotate);
	*/
}

void InputProcess::commandMoveForward(const ecs::Entity &entity)
{
	auto &temporalTransform = ecs::getOrCreate<TemporalTransform>(entity);

	temporalTransform.translation.z -= 1.f;
}

void InputProcess::commandMoveBackward(const ecs::Entity &entity)
{
	auto &temporalTransform = ecs::getOrCreate<TemporalTransform>(entity);

	temporalTransform.translation.z += 1.f;
}

void InputProcess::commandStrafeLeft(const ecs::Entity &entity)
{
	auto &temporalTransform = ecs::getOrCreate<TemporalTransform>(entity);

	temporalTransform.translation.x -= 1.f;
}

void InputProcess::commandStrafeRight(const ecs::Entity &entity)
{
	auto &temporalTransform = ecs::getOrCreate<TemporalTransform>(entity);

	temporalTransform.translation.x += 1.f;
}

void InputProcess::commandMoveUpward(const ecs::Entity &entity)
{
	auto &temporalTransform = ecs::getOrCreate<TemporalTransform>(entity);

	temporalTransform.translation.y += 1.f;
}

void InputProcess::commandMoveDownward(const ecs::Entity &entity)
{
	auto &temporalTransform = ecs::getOrCreate<TemporalTransform>(entity);

	temporalTransform.translation.y -= 1.f;
}


} // proc
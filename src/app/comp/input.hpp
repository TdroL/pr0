#pragma once

#include <core/ecs/component.hpp>
#include <vector>
#include <bitset>

namespace comp
{

struct Input : public ecs::Component<Input>
{
	enum Command {
		COMMAND_NOOP = 0,
		COMMAND_MOVEFORWARD,
		COMMAND_MOVEBACKWARD,
		COMMAND_STRAFELEFT,
		COMMAND_STRAFERIGHT,
		COMMAND_MOVEUPWARD,
		COMMAND_MOVEDOWNWARD,
	};

	enum KeyState {
		KEYSTATE_PRESS = 0,
		KEYSTATE_HOLD,
		KEYSTATE_RELEASE,
		KEYSTATE_ALL
	};

	struct KeyCommand
	{
		size_t key = 0;
		std::bitset<KEYSTATE_ALL> onKeyState{0};
		Command command = COMMAND_NOOP;

		KeyCommand() = default;
		KeyCommand(size_t key, std::bitset<KEYSTATE_ALL> onKeyState, Command command)
			: key{key}, onKeyState{onKeyState}, command{command}
		{}
	};

	std::vector<KeyCommand> keyCommands{};
};

} // comp
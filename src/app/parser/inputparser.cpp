#include "inputparser.hpp"

#include <core/util/str.hpp>
#include <core/ngn/key.hpp>

#include <app/comp/input.hpp>

#include <bitset>

namespace
{

using namespace std;

InputParser inputParser{"input", [] (ecs::Entity &entity, const Parser::objectType &object)
{
	ecs::enable<comp::Input>(entity);

	auto &input = ecs::get<comp::Input>(entity);

	InputParser::enter(object, "keyCommands", [&] (const Parser::objectType &keyCommands)
	{
		if ( ! keyCommands.IsArray())
		{
			return;
		}

		for (rapidjson::SizeType i = 0, s = keyCommands.Size(); i < s; i++)
		{
			const auto &keyCommand = keyCommands[i];

			if ( ! keyCommand.IsArray() || keyCommand.Size() < 2)
			{
				continue;
			}

			const auto &keyName = keyCommand[0];
			const auto &commandName = keyCommand[1];

			if ( ! keyName.IsString() || ! commandName.IsString())
			{
				continue;
			}

			size_t key = 0;
			bitset<comp::Input::KEYSTATE_ALL> onKeyState{0};

			const string &keyNameString = keyName.GetString();

			if (keyNameString.length() > 1)
			{
				char mod = keyNameString[0];

				switch (mod)
				{
					case '*':
					{
						onKeyState.set(comp::Input::KEYSTATE_HOLD);
						key = ngn::key::getKey(keyNameString.substr(1));
						break;
					}
					case '-':
					{
						onKeyState.set(comp::Input::KEYSTATE_RELEASE);
						key = ngn::key::getKey(keyNameString.substr(1));
						break;
					}
					case '/':
					{
						onKeyState.set(comp::Input::KEYSTATE_PRESS);
						onKeyState.set(comp::Input::KEYSTATE_RELEASE);
						key = ngn::key::getKey(keyNameString.substr(1));
						break;
					}
					default:
					{
						onKeyState.set(comp::Input::KEYSTATE_PRESS);
						key = ngn::key::getKey(keyNameString);
					}
				}
			}
			else
			{
				onKeyState.set(comp::Input::KEYSTATE_PRESS);
				key = ngn::key::getKey(keyNameString);
			}

			string command = commandName.GetString();
			util::str::lowercase(command);

			if (command == "moveforward")
			{
				input.keyCommands.emplace_back(key, onKeyState, comp::Input::COMMAND_MOVEFORWARD);
				continue;
			}

			if (command == "movebackward")
			{
				input.keyCommands.emplace_back(key, onKeyState, comp::Input::COMMAND_MOVEBACKWARD);
				continue;
			}

			if (command == "strafeleft")
			{
				input.keyCommands.emplace_back(key, onKeyState, comp::Input::COMMAND_STRAFELEFT);
				continue;
			}

			if (command == "straferight")
			{
				input.keyCommands.emplace_back(key, onKeyState, comp::Input::COMMAND_STRAFERIGHT);
				continue;
			}

			if (command == "moveupward")
			{
				input.keyCommands.emplace_back(key, onKeyState, comp::Input::COMMAND_MOVEUPWARD);
				continue;
			}

			if (command == "movedownward")
			{
				input.keyCommands.emplace_back(key, onKeyState, comp::Input::COMMAND_MOVEDOWNWARD);
				continue;
			}
		}
	});
}};

}
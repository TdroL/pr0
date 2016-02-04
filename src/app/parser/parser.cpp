#include <pch.hpp>

#include "parser.hpp"

#include <core/rn.hpp>

// #include <memory>
// #include <string>
// #include <iostream>

using namespace std;

Parser::Parser(string name, functionType &&parserFunction)
{
	Parser::parsersMap().insert({move(name), move(parserFunction)});
}

map<string, Parser::functionType> & Parser::parsersMap()
{
	static map<string, Parser::functionType> parsers{};

	return parsers;
}

void Parser::parse(ecs::Entity &entity, const Parser::objectType &object)
{
	if ( ! object.IsObject())
	{
		return;
	}

	for (const auto &entry : Parser::parsersMap())
	{
		enter(object, entry.first.c_str(), [&] (const Parser::objectType &member)
		{
			entry.second(entity, member);
		});
	}
}

void Parser::enter(const Parser::objectType &object, const char *name, function<void(const Parser::objectType &)> enterFunction)
{
	if ( ! object.HasMember(name))
	{
		return;
	}

	const Parser::objectType &member = object[name];

	enterFunction(member);
}

template<>
void Parser::assign<string>(const objectType &object, const char *name, string &target)
{
	if ( ! object.HasMember(name))
	{
		return;
	}

	const auto &value = object[name];

	if (value.IsString())
	{
		target = value.GetString();
	}
}

template<>
void Parser::assign<bool>(const objectType &object, const char *name, bool &target)
{
	if ( ! object.HasMember(name))
	{
		return;
	}

	const auto &value = object[name];

	if (value.IsBool())
	{
		target = value.GetBool();
	}
}

template<>
void Parser::assign<float>(const objectType &object, const char *name, float &target)
{
	if ( ! object.HasMember(name))
	{
		return;
	}

	const auto &value = object[name];

	if (value.IsNumber())
	{
		target = static_cast<float>(value.GetDouble());
	}
}

template<>
void Parser::assign<double>(const objectType &object, const char *name, double &target)
{
	if ( ! object.HasMember(name))
	{
		return;
	}

	const auto &value = object[name];

	if (value.IsNumber())
	{
		target = static_cast<double>(value.GetDouble());
	}
}

template<>
void Parser::assign<int>(const objectType &object, const char *name, int &target)
{
	if ( ! object.HasMember(name))
	{
		return;
	}

	const auto &value = object[name];

	if (value.IsNumber())
	{
		target = static_cast<int>(value.GetInt());
	}
}

template<>
void Parser::assign<unsigned int>(const objectType &object, const char *name, unsigned int &target)
{
	if ( ! object.HasMember(name))
	{
		return;
	}

	const auto &value = object[name];

	if (value.IsNumber())
	{
		target = static_cast<unsigned int>(value.GetUint());
	}
}

template<>
void Parser::assign<int64_t>(const objectType &object, const char *name, int64_t &target)
{
	if ( ! object.HasMember(name))
	{
		return;
	}

	const auto &value = object[name];

	if (value.IsNumber())
	{
		target = static_cast<int64_t>(value.GetInt64());
	}
}

template<>
void Parser::assign<uint64_t>(const objectType &object, const char *name, uint64_t &target)
{
	if ( ! object.HasMember(name))
	{
		return;
	}

	const auto &value = object[name];

	if (value.IsNumber())
	{
		target = static_cast<uint64_t>(value.GetUint64());
	}
}

template<>
void Parser::assign<glm::vec2>(const objectType &object, const char *name, glm::vec2 &target)
{
	if ( ! object.HasMember(name))
	{
		return;
	}

	const auto &value = object[name];

	if (value.IsNumber())
	{
		target.x = target.y = static_cast<float>(value.GetDouble());
		return;
	}

	if (value.IsArray())
	{
		size_t size = value.Size();

		if (size == 1)
		{
			const auto &arrayValue = value[0];

			if (arrayValue.IsNumber())
			{
				target.x = target.y = static_cast<float>(arrayValue.GetDouble());
			}

			return;
		}

		if (size > 1)
		{
			const auto &arrayValue0 = value[0];
			const auto &arrayValue1 = value[1];

			if (arrayValue0.IsNumber() && arrayValue1.IsNumber())
			{
				target.x = static_cast<float>(arrayValue0.GetDouble());
				target.y = static_cast<float>(arrayValue1.GetDouble());
			}

			return;
		}

		return;
	}
}

template<>
void Parser::assign<glm::vec3>(const objectType &object, const char *name, glm::vec3 &target)
{
	if ( ! object.HasMember(name))
	{
		return;
	}

	const auto &value = object[name];

	if (value.IsNumber())
	{
		target.x = target.y = target.z = static_cast<float>(value.GetDouble());
		return;
	}

	if (value.IsArray())
	{
		size_t size = value.Size();

		if (size == 1)
		{
			const auto &arrayValue = value[0];

			if (arrayValue.IsNumber())
			{
				target.x = target.y = target.z = static_cast<float>(arrayValue.GetDouble());
			}

			return;
		}

		if (size > 2)
		{
			const auto &arrayValue0 = value[0];
			const auto &arrayValue1 = value[1];
			const auto &arrayValue2 = value[2];

			if (arrayValue0.IsNumber() && arrayValue1.IsNumber() && arrayValue2.IsNumber())
			{
				target.x = static_cast<float>(arrayValue0.GetDouble());
				target.y = static_cast<float>(arrayValue1.GetDouble());
				target.z = static_cast<float>(arrayValue2.GetDouble());
			}

			return;
		}

		return;
	}
}

template<>
void Parser::assign<glm::vec4>(const objectType &object, const char *name, glm::vec4 &target)
{
	if ( ! object.HasMember(name))
	{
		return;
	}

	const auto &value = object[name];

	if (value.IsNumber())
	{
		target.x = target.y = target.z = target.w = static_cast<float>(value.GetDouble());
		return;
	}

	if (value.IsArray())
	{
		size_t size = value.Size();

		if (size == 1)
		{
			const auto &arrayValue = value[0];

			if (arrayValue.IsNumber())
			{
				target.x = target.y = target.z = target.w = static_cast<float>(arrayValue.GetDouble());
			}

			return;
		}

		if (size > 3)
		{
			const auto &arrayValue0 = value[0];
			const auto &arrayValue1 = value[1];
			const auto &arrayValue2 = value[2];
			const auto &arrayValue3 = value[3];

			if (arrayValue0.IsNumber() && arrayValue1.IsNumber() && arrayValue2.IsNumber() && arrayValue3.IsNumber())
			{
				target.x = static_cast<float>(arrayValue0.GetDouble());
				target.y = static_cast<float>(arrayValue1.GetDouble());
				target.z = static_cast<float>(arrayValue2.GetDouble());
				target.w = static_cast<float>(arrayValue3.GetDouble());
			}

			return;
		}

		return;
	}
}
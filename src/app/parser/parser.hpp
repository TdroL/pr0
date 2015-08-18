#ifndef APP_PARSER_HPP
#define APP_PARSER_HPP

#include <core/ecs/entity.hpp>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <map>
#include <functional>
#include <string>

class Parser
{
public:
	typedef rapidjson::Value objectType;
	typedef std::function<void(ecs::Entity &, const objectType &)> functionType;

	Parser(std::string name, functionType &&parserFunction);

	static std::map<std::string, functionType> & parsersMap();

	static void parse(ecs::Entity &entity, const objectType &object);

	static void enter(const objectType &object, const char *name, std::function<void(const objectType&)> enterFunction);

	template<typename T>
	static void assign(const objectType &object, const char *name, T &target);
};

#endif
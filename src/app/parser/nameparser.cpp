#include <pch.hpp>

#include "nameparser.hpp"

#include <app/comp/name.hpp>

namespace
{

NameParser nameParser{"name", [] (ecs::Entity &entity, const Parser::objectType &object)
{
	ecs::enable<comp::Name>(entity);

	auto &name = ecs::get<comp::Name>(entity);

	NameParser::assign(object, "name", name.name);
}};

}
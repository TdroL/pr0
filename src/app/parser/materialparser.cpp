#include "materialparser.hpp"

#include <app/comp/material.hpp>

namespace
{

MaterialParser materialParser{"material", [] (ecs::Entity &entity, const Parser::objectType &object)
{
	ecs::enable<comp::Material>(entity);

	auto &material = ecs::get<comp::Material>(entity);

	MaterialParser::assign(object, "diffuse", material.diffuse);
	MaterialParser::assign(object, "shininess", material.shininess);
}};

}
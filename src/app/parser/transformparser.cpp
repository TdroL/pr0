#include <pch.hpp>

#include "transformparser.hpp"

#include <app/comp/transform.hpp>

// #include <iostream>

namespace
{

using namespace std;

TransformParser transformParser{"transform", [] (ecs::Entity &entity, const Parser::objectType &object)
{
	ecs::enable<comp::Transform>(entity);

	auto &transform = ecs::get<comp::Transform>(entity);

	TransformParser::assign(object, "translation", transform.translation);
	TransformParser::assign(object, "rotation", transform.rotation);
	TransformParser::assign(object, "scale", transform.scale);
}};

}
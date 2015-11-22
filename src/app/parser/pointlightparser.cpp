#include <pch.hpp>

#include "pointlightparser.hpp"

#include <app/comp/pointlight.hpp>

// #include <iostream>

namespace
{

using namespace std;

PointLightParser pointLightParser{"pointLight", [] (ecs::Entity &entity, const Parser::objectType &object)
{
	ecs::enable<comp::PointLight>(entity);

	auto &pointLight = ecs::get<comp::PointLight>(entity);

	PointLightParser::assign(object, "color", pointLight.color);
	PointLightParser::assign(object, "intensity", pointLight.intensity);
	PointLightParser::assign(object, "radius", pointLight.radius);
	PointLightParser::assign(object, "cutoff", pointLight.cutoff);
}};

}
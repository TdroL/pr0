#include "temporaltransformparser.hpp"

#include <app/comp/temporaltransform.hpp>

#include <iostream>

namespace
{

using namespace std;

TemporalTransformParser temporaltransformParser{"temporaltransform", [] (ecs::Entity &entity, const Parser::objectType &object)
{
	ecs::enable<comp::TemporalTransform>(entity);

	auto &temporalTransform = ecs::get<comp::TemporalTransform>(entity);

	TemporalTransformParser::assign(object, "translationSpeed", temporalTransform.translationSpeed);
	TemporalTransformParser::assign(object, "translationNormalized", temporalTransform.translationNormalized);
	TemporalTransformParser::assign(object, "rotationSpeed", temporalTransform.rotationSpeed);
}};

}
#include <pch.hpp>

#include "occluderparser.hpp"

#include <app/comp/occluder.hpp>

namespace
{

OccluderParser occluderParser{"occluder", [] (ecs::Entity &entity, const Parser::objectType &)
{
	ecs::enable<comp::Occluder>(entity);
}};

}
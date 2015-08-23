#include <pch.hpp>

#include "shadingparser.hpp"

#include <app/comp/shading.hpp>

// #include <string>

namespace
{

using namespace std;

ShadingParser shadingParser{"shading", [] (ecs::Entity &entity, const Parser::objectType &object)
{
	ecs::enable<comp::Shading>(entity);

	auto &shading = ecs::get<comp::Shading>(entity);

	ShadingParser::enter(object, "group", [&] (const Parser::objectType &group)
	{
		if (group.IsString())
		{
			const string refValue = group.GetString();

			if (refValue == "shaded")
			{
				shading.group = comp::Shading::GROUP_SHADED;
			}
			else if (refValue == "flat")
			{
				shading.group = comp::Shading::GROUP_FLAT;
			}
			else
			{
				shading.group = comp::Shading::GROUP_NONE;
			}
		}
	});
}};

}
#include <pch.hpp>

#include "stencilparser.hpp"

#include <app/comp/stencil.hpp>

// #include <string>

namespace
{

using namespace std;

StencilParser stencilParser{"stencil", [] (ecs::Entity &entity, const Parser::objectType &object)
{
	ecs::enable<comp::Stencil>(entity);

	auto &stencil = ecs::get<comp::Stencil>(entity);

	StencilParser::enter(object, "ref", [&] (const Parser::objectType &ref)
	{
		if (ref.IsString())
		{
			const string refValue = ref.GetString();

			if (refValue == "shaded")
			{
				stencil.ref = comp::Stencil::MASK_SHADED;
			}
			else if (refValue == "flat")
			{
				stencil.ref = comp::Stencil::MASK_FLAT;
			}
		}
		else if (ref.IsUint())
		{
			stencil.ref = ref.GetUint();
		}
	});
}};

}
#pragma once

#include <core/ecs/entity.hpp>
#include <core/rn.hpp>
#include <core/rn/program.hpp>

namespace proc
{

class MeshRenderer
{
public:
	// static void renderZ(const ecs::Entity &entity, rn::Program &prog, GLint locationM);
	// static void render(const ecs::Entity &entity, rn::Program &prog);
	static void render(const ecs::Entity &entity);
};

} // proc
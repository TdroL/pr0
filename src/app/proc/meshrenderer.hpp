#ifndef APP_PROC_MESHRENDERER_HPP
#define APP_PROC_MESHRENDERER_HPP

#include <core/ecs/entity.hpp>
#include <glm/glm.hpp>
#include <core/rn/program.hpp>

namespace proc
{

class MeshRenderer
{
public:
	static void render(const ecs::Entity &entity, rn::Program &prog);
};

} // proc

#endif
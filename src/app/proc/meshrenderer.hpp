#ifndef APP_PROC_MESHRENDERER_HPP
#define APP_PROC_MESHRENDERER_HPP

#include <core/ecs/entity.hpp>
#include <glm/glm.hpp>
#include <core/gl/program.hpp>

namespace proc
{

class MeshRenderer
{
public:
	static void render(const ecs::Entity &entity, gl::Program &prog);
};

} // proc

#endif
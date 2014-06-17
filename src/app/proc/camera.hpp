#ifndef APP_PROC_CAMERA_HPP
#define APP_PROC_CAMERA_HPP

#include <core/ecs/entity.hpp>
#include <glm/glm.hpp>

namespace proc
{

class Camera
{
public:
	static void update(const ecs::Entity &entity, const glm::vec3 &translate, const glm::vec3 &rotate);
};

} // proc

#endif
#ifndef PHS_SPHERE_HPP
#define PHS_SPHERE_HPP

#include <glm/glm.hpp>

namespace phs
{

class Sphere {
public:
	glm::vec3 position{0.f};
	float radius = 0.f;

	Sphere() = default;

	Sphere(const glm::mat4 &WVP, bool needInverse = true);
};

} // phs

#endif
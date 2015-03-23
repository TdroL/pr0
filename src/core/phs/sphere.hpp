#ifndef PHS_SPHERE_HPP
#define PHS_SPHERE_HPP

#include <glm/glm.hpp>

namespace phs
{

class Sphere {
public:
	glm::vec3 pos{0.f};
	float radius = 0.f;
};

} // phs

#endif
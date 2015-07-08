#ifndef PHS_PLANE_HPP
#define PHS_PLANE_HPP

#include "aabb.hpp"
#include "point.hpp"
#include "sphere.hpp"

#include <glm/glm.hpp>

namespace phs
{

class Plane {
public:
	glm::vec3 normal{0.f};
	float d = 0.f;

	Plane() = default;

	explicit Plane(const glm::vec4 &vec);

	Plane & operator=(const glm::vec4 &vec);

	float distance(const Point &point) const;

	float distance(const Sphere &sphere) const;

	void normalize();
};

} // phs

#endif
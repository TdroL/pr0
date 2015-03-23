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

	float distance(const AABB &aabb) const
	{
		return 0.f;
	}

	float distance(const Point &point) const
	{
		return glm::dot(point.pos, normal) + d;
	}

	float distance(const Sphere &sphere) const
	{
		return glm::dot(sphere.pos, normal) + d + sphere.radius;
	}

	void normalize()
	{
		float invLength = glm::inversesqrt(glm::dot(normal, normal));

		normal *= invLength;
		d *= invLength;
	}

	Plane & operator=(const glm::vec4 &vec)
	{
		normal.x = vec.x;
		normal.y = vec.y;
		normal.z = vec.z;
		d = vec.w;

		normalize();

		return *this;
	}
};

} // phs

#endif
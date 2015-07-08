#include "plane.hpp"

namespace phs
{

Plane::Plane(const glm::vec4 &vec)
{
	normal.x = vec.x;
	normal.y = vec.y;
	normal.z = vec.z;
	d = vec.w;

	normalize();
}

Plane & Plane::operator=(const glm::vec4 &vec)
{
	normal.x = vec.x;
	normal.y = vec.y;
	normal.z = vec.z;
	d = vec.w;

	normalize();

	return *this;
}

float Plane::distance(const Point &point) const
{
	return glm::dot(point.position, normal) + d;
}

float Plane::distance(const Sphere &sphere) const
{
	return glm::dot(sphere.position, normal) + d + sphere.radius;
}

void Plane::normalize()
{
	float invLength = glm::inversesqrt(glm::dot(normal, normal));

	normal *= invLength;
	d *= invLength;
}

} // phs
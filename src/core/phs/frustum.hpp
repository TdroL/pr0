#ifndef PHS_FRUSTUM_HPP
#define PHS_FRUSTUM_HPP

#include "sphere.hpp"
#include "aabb.hpp"
#include "point.hpp"
#include "plane.hpp"

#include <glm/glm.hpp>

namespace phs
{

class Frustum {
public:
	Plane planes[6];

	explicit Frustum(const glm::mat4 &WVP);
	Frustum(const glm::mat4 &P, const glm::mat4 &V);

	bool test(const Sphere &sphere) const;
	bool test(const AABB &aabb) const;
	bool test(const Point &point) const;

	float distance(const Sphere &sphere) const;
	float distance(const Point &point) const;
};

} // phs

#endif
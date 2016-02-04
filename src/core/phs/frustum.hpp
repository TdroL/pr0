#pragma once

#include "sphere.hpp"
#include "aabb.hpp"
#include "point.hpp"
#include "plane.hpp"
#include "../rn.hpp"

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
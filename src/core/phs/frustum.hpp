#ifndef PHS_FRUSTUM_HPP
#define PHS_FRUSTUM_HPP

#include "sphere.hpp"
#include "aabb.hpp"
#include "point.hpp"
#include "plane.hpp"

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>

namespace phs
{

class Frustum {
public:
	Plane planes[6];

	explicit Frustum(const glm::mat4 &WVP)
	{
		glm::vec4 row[4] = {
			glm::row(WVP, 0),
			glm::row(WVP, 1),
			glm::row(WVP, 2),
			glm::row(WVP, 3)
		};

		planes[0] = row[3] - row[0];
		planes[1] = row[3] + row[0];
		planes[2] = row[3] - row[1];
		planes[3] = row[3] + row[1];
		planes[4] = row[3] - row[2];
		planes[5] = row[3] + row[2];
	}

	bool test(const Sphere &sphere) const
	{
		for (const auto &plane : planes)
		{
			if (plane.distance(sphere) < 0.f)
			{
				return false;
			}
		}

		return true;
	}

	bool test(const AABB &aabb) const
	{
		return false;
	}

	bool test(const Point &point) const
	{
		for (const auto &plane : planes)
		{
			if (plane.distance(point) < 0.f)
			{
				return false;
			}
		}

		return true;
	}

	float distance(const Sphere &sphere) const
	{
		float d = planes[0].distance(sphere);
		d = std::min(d, planes[1].distance(sphere));
		d = std::min(d, planes[2].distance(sphere));
		d = std::min(d, planes[3].distance(sphere));
		d = std::min(d, planes[4].distance(sphere));
		d = std::min(d, planes[5].distance(sphere));

		return d;
	}

	float distance(const Point &point) const
	{
		float d = planes[0].distance(point);
		d = std::min(d, planes[1].distance(point));
		d = std::min(d, planes[2].distance(point));
		d = std::min(d, planes[3].distance(point));
		d = std::min(d, planes[4].distance(point));
		d = std::min(d, planes[5].distance(point));

		return d;
	}
};

} // phs

#endif
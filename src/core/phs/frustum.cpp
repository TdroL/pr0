#include <pch.hpp>

#include "frustum.hpp"

#include <glm/gtc/matrix_access.hpp>

#include <algorithm>

namespace phs
{

using namespace std;

Frustum::Frustum(const glm::mat4 &WVP)
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

Frustum::Frustum(const glm::mat4 &P, const glm::mat4 &V)
{
	glm::mat4 WVP = P * V;
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

bool Frustum::test(const Sphere &sphere) const
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

bool Frustum::test(const AABB &aabb) const
{
	for (const auto &plane : planes)
	{
		int out = 0;
		int in = 0;

		const Point boxCorners[] = {
			Point{aabb.max.x, aabb.max.y, aabb.max.z},
			Point{aabb.min.x, aabb.max.y, aabb.max.z},
			Point{aabb.max.x, aabb.min.y, aabb.max.z},
			Point{aabb.min.x, aabb.min.y, aabb.max.z},
			Point{aabb.max.x, aabb.max.y, aabb.min.z},
			Point{aabb.min.x, aabb.max.y, aabb.min.z},
			Point{aabb.max.x, aabb.min.y, aabb.min.z},
			Point{aabb.min.x, aabb.min.y, aabb.min.z}
		};

		for (int i = 0; i < 8 && (in == 0 || out == 0); i++)
		{
			if (plane.distance(boxCorners[i]) < 0.f)
			{
				out++;
			}
			else
			{
				in++;
			}
		}

		if (in == 0)
		{
			return false;
		}
		else if (out != 0)
		{
			return false;
		}
	}

	return true;
}

bool Frustum::test(const Point &point) const
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

float Frustum::distance(const Sphere &sphere) const
{
	float d = planes[0].distance(sphere);

	d = min(d, planes[1].distance(sphere));
	d = min(d, planes[2].distance(sphere));
	d = min(d, planes[3].distance(sphere));
	d = min(d, planes[4].distance(sphere));
	d = min(d, planes[5].distance(sphere));

	return d;
}

float Frustum::distance(const Point &point) const
{
	float d = planes[0].distance(point);

	d = min(d, planes[1].distance(point));
	d = min(d, planes[2].distance(point));
	d = min(d, planes[3].distance(point));
	d = min(d, planes[4].distance(point));
	d = min(d, planes[5].distance(point));

	return d;
}

} // phs
#include <pch.hpp>

#include "raster.hpp"
#include <glm/glm.hpp>
#include <cstdint>
#include <iostream>
#include <iomanip>

namespace cull
{

void Raster::reset(const glm::ivec2 &dim)
{
	this->dim = dim;

	data.reset(new float[dim.x * dim.y]);
}


void Raster::clear()
{
	std::fill(data.get(), data.get() + (dim.x * dim.y), 1.f);
}

void Raster::draw(const glm::vec3 (&polygon)[3])
{
	glm::vec3 v1 = polygon[0] * glm::vec3{0.5f, 0.5f, 1.f} + glm::vec3{0.5f, 0.5f, 0.f};
	glm::vec3 v2 = polygon[1] * glm::vec3{0.5f, 0.5f, 1.f} + glm::vec3{0.5f, 0.5f, 0.f};
	glm::vec3 v3 = polygon[2] * glm::vec3{0.5f, 0.5f, 1.f} + glm::vec3{0.5f, 0.5f, 0.f};
	glm::vec3 v4;

	// sort vertices by "y"
	if (v2.y < v1.y)
	{
		std::swap(v2, v1);
	}

	if (v3.y < v1.y)
	{
		std::swap(v3, v1);
	}

	if (v3.y < v2.y)
	{
		std::swap(v3, v2);
	}

	// calculate v4 [v4.y == v2.y]
	float a = (v3.y - v2.y) / (v3.y - v1.y);
	v4 = glm::mix(v3, v1, a);

	v1 *= glm::vec3{dim, 1.f};
	v2 *= glm::vec3{dim, 1.f};
	v3 *= glm::vec3{dim, 1.f};
	v4 *= glm::vec3{dim, 1.f};

	/**
	 *  1
	 *  |\
	 *  | \
	 *  4--2
	 *  | /
	 *  |/
	 *  3
	 */

	// draw top triangle
	if (v2.y != v1.y)
	{
		float xSlope1 = (v2.x - v1.x) / (v2.y - v1.y);
		float xSlope2 = (v4.x - v1.x) / (v2.y - v1.y);

		float zSlope1 = (v2.z - v1.z) / (v2.y - v1.y);
		float zSlope2 = (v4.z - v1.z) / (v2.y - v1.y);

		float x1 = v1.x;
		float x2 = v1.x;
		float z1 = v1.z;
		float z2 = v1.z;

		for (int32_t scanlineY = v1.y, scanlineEnd = glm::min(static_cast<int32_t>(v2.y), dim.y - 1); scanlineY <= scanlineEnd; scanlineY++)
		{
			if (scanlineY >= 0)
			{
				int32_t xStart;
				int32_t xEnd;
				float zStart;
				float zEnd;

				if (x1 <= x2)
				{
					xStart = x1;
					xEnd = x2;
					zStart = z1;
					zEnd = z2;
				}
				else
				{
					xStart = x2;
					xEnd = x1;
					zStart = z2;
					zEnd = z1;
				}

				if (xEnd == xStart)
				{
					int32_t idx = scanlineY * dim.x + xStart;

					if (zStart < data[idx])
					{
						data[idx] = zStart;
					}
				}
				else
				{
					float zStep = 1.f / static_cast<float>(xEnd - xStart);
					int32_t line = scanlineY * dim.x;
					for (int32_t i = glm::max(xStart, 0), j = 0, k = glm::min(xEnd, dim.x); i <= k; i++, j++)
					{
						float z = glm::mix(zStart, zEnd, zStep * j);
						int32_t idx = line + i;
						if (z < data[idx])
						{
							data[idx] = z;
						}
					}
				}
			}

			x1 += xSlope1;
			x2 += xSlope2;

			z1 += zSlope1;
			z2 += zSlope2;
		}
	}

	// draw bottom triangle
	if (v3.y != v2.y)
	{
		float xSlope1 = (v3.x - v2.x) / (v3.y - v2.y);
		float xSlope2 = (v3.x - v4.x) / (v3.y - v2.y);

		float zSlope1 = (v3.z - v2.z) / (v3.y - v2.y);
		float zSlope2 = (v3.z - v4.z) / (v3.y - v2.y);

		float x1 = v3.x;
		float x2 = v3.x;
		float z1 = v3.z;
		float z2 = v3.z;

		for (int32_t scanlineY = v3.y, scanlineEnd = glm::max(static_cast<int32_t>(v2.y), -1); scanlineY > scanlineEnd; scanlineY--)
		{
			if (scanlineY >= dim.y)
			{
				x1 -= xSlope1;
				x2 -= xSlope2;

				z1 -= zSlope1;
				z2 -= zSlope2;
			}

			int32_t xStart;
			int32_t xEnd;
			float zStart;
			float zEnd;

			if (x1 <= x2)
			{
				xStart = x1;
				xEnd = x2;
				zStart = z1;
				zEnd = z2;
			}
			else
			{
				xStart = x2;
				xEnd = x1;
				zStart = z2;
				zEnd = z1;
			}

			if (xEnd == xStart)
			{
				int32_t idx = scanlineY * dim.x + xStart;
				if (zStart < data[idx])
				{
					data[idx] = zStart;
				}
			}
			else
			{
				float zStep = 1.f / static_cast<float>(xEnd - xStart);
				int32_t line = scanlineY * dim.x;
				for (int32_t i = glm::max(xStart, 0), j = 0, k = glm::min(xEnd, dim.x); i <= k; i++, j++)
				{
					float z = glm::mix(zStart, zEnd, zStep * j);
					int32_t idx = line + i;
					if (z < data[idx])
					{
						data[idx] = z;
					}
				}
			}
		}
	}
}

void Raster::test()
{
	//
}

} // cull
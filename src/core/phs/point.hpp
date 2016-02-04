#pragma once

#include "../rn.hpp"

namespace phs
{

class Point {
public:
	glm::vec3 position{0.f};

	Point() = default;
	explicit Point(const glm::vec3 &position)
		: position{position}
	{}

	Point(float x, float y, float z)
		: position{x, y, z}
	{}
};

} // phs
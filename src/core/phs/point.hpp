#ifndef PHS_POINT_HPP
#define PHS_POINT_HPP

#include <glm/glm.hpp>

namespace phs
{

class Point {
public:
	glm::vec3 pos{0.f};

	Point() = default;
	explicit Point(const glm::vec3 &pos)
		: pos{pos}
	{}

	explicit Point(float x, float y, float z)
		: pos{x, y, z}
	{}
};

} // phs

#endif
#ifndef PHS_AABB_HPP
#define PHS_AABB_HPP

#include <cmath>
#include <glm/glm.hpp>

namespace phs
{

class AABB {
public:
	glm::vec3 min{};
	glm::vec3 max{};

	void extend(AABB &aabb)
	{
		min.x = std::min({ min.x, aabb.min.x, aabb.max.x });
		min.y = std::min({ min.y, aabb.min.y, aabb.max.y });
		min.z = std::min({ min.z, aabb.min.z, aabb.max.z });
		max.x = std::max({ max.x, aabb.max.x, aabb.min.x });
		max.y = std::max({ max.y, aabb.max.y, aabb.min.y });
		max.z = std::max({ max.z, aabb.max.z, aabb.min.z });
	}
};

} // phs

#endif
#ifndef PHS_AABB_HPP
#define PHS_AABB_HPP

#include <limits>
#include <glm/glm.hpp>

namespace phs
{

class AABB {
public:
	glm::vec3 min{std::numeric_limits<float>::max()};
	glm::vec3 max{-std::numeric_limits<float>::max()};

	AABB() = default;

	AABB(const glm::vec3 &min, const glm::vec3 &max)
		: min{min}, max{max}
	{}

	AABB(const glm::mat4 &WVP, bool needInverse = true);

	void extend(const AABB &aabb);
};

} // phs

#endif
#ifndef PHS_AABB_HPP
#define PHS_AABB_HPP

#include <glm/glm.hpp>

namespace phs
{

class AABB {
public:
	glm::vec3 min{};
	glm::vec3 max{};
};

} // phs

#endif
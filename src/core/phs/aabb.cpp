#include <pch.hpp>

#include "aabb.hpp"
#include <algorithm>

namespace phs
{

AABB::AABB(const glm::mat4 &WVP, bool needInverse)
{
	const auto invWVP = needInverse ? glm::inverse(WVP) : WVP;

	glm::vec4 corners[] = {
		invWVP * glm::vec4{-1.f,  1.f, -1.f, 1.f},
		invWVP * glm::vec4{ 1.f,  1.f, -1.f, 1.f},
		invWVP * glm::vec4{ 1.f, -1.f, -1.f, 1.f},
		invWVP * glm::vec4{-1.f, -1.f, -1.f, 1.f},
		invWVP * glm::vec4{-1.f,  1.f,  1.f, 1.f},
		invWVP * glm::vec4{ 1.f,  1.f,  1.f, 1.f},
		invWVP * glm::vec4{ 1.f, -1.f,  1.f, 1.f},
		invWVP * glm::vec4{-1.f, -1.f,  1.f, 1.f}
	};

	for (auto &c : corners)
	{
		c /= c.w;

		min.x = std::min(min.x, c.x);
		min.y = std::min(min.y, c.y);
		min.z = std::min(min.z, c.z);
		max.x = std::max(max.x, c.x);
		max.y = std::max(max.y, c.y);
		max.z = std::max(max.z, c.z);
	}
}

void AABB::extend(const AABB &aabb)
{
	min.x = std::min({ min.x, aabb.min.x, aabb.max.x });
	min.y = std::min({ min.y, aabb.min.y, aabb.max.y });
	min.z = std::min({ min.z, aabb.min.z, aabb.max.z });
	max.x = std::max({ max.x, aabb.max.x, aabb.min.x });
	max.y = std::max({ max.y, aabb.max.y, aabb.min.y });
	max.z = std::max({ max.z, aabb.max.z, aabb.min.z });
}

} // phs
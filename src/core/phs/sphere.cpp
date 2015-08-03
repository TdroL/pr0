#include "sphere.hpp"

#include <array>

#include <minball/minball.hpp>

namespace phs
{

Minball minball{8};

Sphere::Sphere(const glm::mat4 &WVP, bool needInverse)
{
	const auto invWVP = needInverse ? glm::inverse(WVP) : WVP;

	glm::vec4 corners[8] = {
		invWVP * glm::vec4{ 1.f,  1.f,  1.f, 1.f},
		invWVP * glm::vec4{ 1.f,  1.f, -1.f, 1.f},
		invWVP * glm::vec4{ 1.f, -1.f,  1.f, 1.f},
		invWVP * glm::vec4{ 1.f, -1.f, -1.f, 1.f},
		invWVP * glm::vec4{-1.f,  1.f,  1.f, 1.f},
		invWVP * glm::vec4{-1.f,  1.f, -1.f, 1.f},
		invWVP * glm::vec4{-1.f, -1.f,  1.f, 1.f},
		invWVP * glm::vec4{-1.f, -1.f, -1.f, 1.f}
	};

	for (size_t i = 0; i < 8; i++)
	{
		auto &c = corners[i];
		c /= c.w;

		minball.setPoint(i, c.x, c.y, c.z);
	}

	std::array<float, 3> center = minball.center();

	position.x = center[0];
	position.y = center[1];
	position.z = center[2];
	radius = minball.radius();
}

} // phs
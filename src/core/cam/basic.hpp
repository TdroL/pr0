#ifndef CAM_BASIC_HPP
#define CAM_BASIC_HPP

#include <type_traits>
#include <glm/glm.hpp>

namespace cam {

class Basic
{
public:
	glm::vec3 position{0.0f, 0.0f, 0.0f};
	glm::vec3 rotation{0.0f, 0.0f, 0.0f};

	glm::mat4 viewMatrix{1.f};

	Basic() = default;
	Basic(const glm::vec3 &position, const glm::vec3 &rotation = glm::vec3{0.0f, 0.0f, 0.0f})
		: position{position}, rotation{rotation}
	{
		recalc();
	}

	void update(const glm::vec3 &rotate, const glm::vec3 &translate);
	void updateRecalc(const glm::vec3 &rotate, const glm::vec3 &translate);

	void recalc();
};

} // cam

#endif
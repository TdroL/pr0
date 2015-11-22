#pragma once

#include <core/ecs/component.hpp>
#include <glm/glm.hpp>

namespace comp
{

struct TemporalTransform : public ecs::Component<TemporalTransform>
{
	glm::vec3 translation{0.0};
	glm::vec3 translationSpeed{10.0};
	bool translationNormalized = true;

	glm::vec3 rotation{0.0};
	glm::vec3 rotationSpeed{90.0};
};

} // comp
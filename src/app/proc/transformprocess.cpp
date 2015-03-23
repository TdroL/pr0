#include "transformprocess.hpp"

#include <core/ngn.hpp>

#include <app/comp/boundingobject.hpp>
#include <app/comp/transform.hpp>
#include <app/comp/temporaltransform.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

#include <cmath>

namespace proc
{

using namespace comp;

void TransformProcess::update(const ecs::Entity &entity)
{
	auto &temporalTransform = ecs::get<TemporalTransform>(entity);
	auto &transform = ecs::get<Transform>(entity);

	{
		if (temporalTransform.translation != glm::vec3{0.0})
		{
			if (temporalTransform.translationNormalized)
			{
				temporalTransform.translation = glm::normalize(temporalTransform.translation);
			}

			transform.translation += temporalTransform.translation * temporalTransform.translationSpeed * static_cast<float>(ngn::dt);
			temporalTransform.translation = glm::vec3{0.0};

			if (ecs::has<BoundingObject>(entity))
			{
				ecs::get<BoundingObject>(entity).dirty = true;
			}
		}

	}
}

} // proc
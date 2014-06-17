#include "camera.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <app/comp/position.hpp>
#include <app/comp/rotation.hpp>
#include <app/comp/view.hpp>


namespace proc
{

using namespace comp;

void Camera::update(const ecs::Entity &entity, const glm::vec3 &translate, const glm::vec3 &rotate)
{
	Rotation &rotation = ecs::get<Rotation>(entity);
	Position &position = ecs::get<Position>(entity);
	View &view = ecs::get<View>(entity);

	// translate and rotate camera (fps style)
	{
		rotation.x += rotate.x;
		rotation.y += rotate.y;
		rotation.z += rotate.z;

		rotation.x = glm::clamp(rotation.x, -90.f, 90.f);
		rotation.y = std::fmod(rotation.y, 360.f);
		rotation.z = std::fmod(rotation.z, 360.f);

		if (translate.x != 0 || translate.y != 0 || translate.z != 0)
		{
			glm::vec3 translateRotate{rotation.x, rotation.y, rotation.z};
			translateRotate *= glm::radians(0.5);

			float sinX = std::sin(translateRotate.x);
			float cosX = std::cos(translateRotate.x);

			float sinY = std::sin(translateRotate.y);
			float cosY = std::cos(translateRotate.y);

			float sinZ = std::sin(translateRotate.z);
			float cosZ = std::cos(translateRotate.z);

			glm::quat orientX{cosX, sinX, 0.f, 0.f};
			glm::quat orientY{cosY, 0.f, sinY, 0.f};
			glm::quat orientZ{cosZ, 0.f, 0.f, sinZ};

			glm::quat orient = glm::normalize(orientZ * orientY * orientX);

			glm::quat rotatedQuat = orient * glm::quat{0.f, translate} * glm::conjugate(orient);

			position.x += rotatedQuat.x;
			position.y += rotatedQuat.y;
			position.z += rotatedQuat.z;
		}
	}

	// update view matrix
	{
		glm::vec3 viewRotation{rotation.x, rotation.y, rotation.z};
		viewRotation *= -glm::radians(0.5);

		float sinX = std::sin(viewRotation.x);
		float cosX = std::cos(viewRotation.x);

		float sinY = std::sin(viewRotation.y);
		float cosY = std::cos(viewRotation.y);

		float sinZ = std::sin(viewRotation.z);
		float cosZ = std::cos(viewRotation.z);

		glm::quat orientX{cosX, sinX, 0.f, 0.f};
		glm::quat orientY{cosY, 0.f, sinY, 0.f};
		glm::quat orientZ{cosZ, 0.f, 0.f, sinZ};

		glm::quat orientation = glm::normalize(orientX * orientY * orientZ);

		view.matrix = glm::translate(glm::mat4_cast(orientation), -glm::vec3{position.x, position.y, position.z});
	}
}

} // proc
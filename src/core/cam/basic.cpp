#include "basic.hpp"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace cam
{

using namespace std;

const double PI = 3.141592653589793238462643383279;
const double TAU = 2.0 * PI;
const double RAD = TAU / 360.0;
const double DEG = 1.0 / TAU;

void Basic::update(const glm::vec3 &rotate, const glm::vec3 &translate)
{
	rotation += rotate;

	rotation.x = glm::clamp(rotation.x, -90.f, 90.f);
	rotation.y = fmod(rotation.y, 360.f);
	rotation.z = fmod(rotation.z, 360.f);

	glm::vec3 translateRotate{rotation};
	translateRotate *= RAD * 0.5;

	float sinX = sin(translateRotate.x);
	float cosX = cos(translateRotate.x);

	float sinY = sin(translateRotate.y);
	float cosY = cos(translateRotate.y);

	float sinZ = sin(translateRotate.z);
	float cosZ = cos(translateRotate.z);

	glm::quat orientX{cosX, sinX, 0.f, 0.f};
	glm::quat orientY{cosY, 0.f, sinY, 0.f};
	glm::quat orientZ{cosZ, 0.f, 0.f, sinZ};

	glm::quat orient = glm::normalize(orientZ * orientY * orientX);

	glm::quat rotatedQuat = orient * glm::quat{0.f, translate} * glm::conjugate(orient);

	position.x += rotatedQuat.x;
	position.y += rotatedQuat.y;
	position.z += rotatedQuat.z;
}

void Basic::updateRecalc(const glm::vec3 &rotate, const glm::vec3 &translate)
{
	update(rotate, translate);
	recalc();
}

void Basic::recalc()
{
	glm::vec3 viewRotation = rotation;
	viewRotation *= -RAD * 0.5;

	float sinX = sin(viewRotation.x);
	float cosX = cos(viewRotation.x);

	float sinY = sin(viewRotation.y);
	float cosY = cos(viewRotation.y);

	float sinZ = sin(viewRotation.z);
	float cosZ = cos(viewRotation.z);

	glm::quat orientX{cosX, sinX, 0.f, 0.f};
	glm::quat orientY{cosY, 0.f, sinY, 0.f};
	glm::quat orientZ{cosZ, 0.f, 0.f, sinZ};

	glm::quat orientation = glm::normalize(orientX * orientY * orientZ);

	viewMatrix = glm::translate(glm::mat4_cast(orientation), -position);
}

}
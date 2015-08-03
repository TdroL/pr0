#include "rebuildboundingobjectprocess.hpp"

#include <app/comp/boundingobject.hpp>
#include <app/comp/transform.hpp>
#include <app/comp/mesh.hpp>

#include <core/asset/mesh.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace proc
{

using namespace std;
using namespace comp;

void RebuildBoundingObjectProcess::update(const ecs::Entity &entity)
{
	auto &boundingObject = ecs::getOrCreate<BoundingObject>(entity);

	if ( ! boundingObject.dirty)
	{
		return;
	}

	glm::mat4 M{1.f};

	auto &transform = ecs::get<Transform>(entity);

	M = glm::translate(M, transform.translation);
	M = glm::rotate(M, glm::radians(transform.rotation.x), glm::vec3{1.f, 0.f, 0.f});
	M = glm::rotate(M, glm::radians(transform.rotation.y), glm::vec3{0.f, 1.f, 0.f});
	M = glm::rotate(M, glm::radians(transform.rotation.z), glm::vec3{0.f, 0.f, 1.f});
	M = glm::scale(M, transform.scale);

	auto &mesh = asset::mesh::get(ecs::get<Mesh>(entity).id);

	glm::vec4 corners[2] {
		glm::vec4{ mesh.boundingBox.min, 1.f },
		glm::vec4{ mesh.boundingBox.max, 1.f }
	};

	corners[0] = M * corners[0];
	corners[1] = M * corners[1];

	boundingObject.aabb.min.x = min(corners[0].x, corners[1].x);
	boundingObject.aabb.max.x = max(corners[0].x, corners[1].x);
	boundingObject.aabb.min.y = min(corners[0].y, corners[1].y);
	boundingObject.aabb.max.y = max(corners[0].y, corners[1].y);
	boundingObject.aabb.min.z = min(corners[0].z, corners[1].z);
	boundingObject.aabb.max.z = max(corners[0].z, corners[1].z);

	boundingObject.sphere.position = transform.translation;
	boundingObject.sphere.radius = mesh.boundingRadius * max({transform.scale.x, transform.scale.y, transform.scale.z});

	boundingObject.dirty = false;
}

} // proc
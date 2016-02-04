#include <pch.hpp>

#include "rebuildboundingvolumeprocess.hpp"

#include <app/comp/boundingvolume.hpp>
#include <app/comp/transform.hpp>
#include <app/comp/mesh.hpp>

#include <core/asset/mesh.hpp>

// #include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace proc
{

using namespace std;
using namespace comp;

void RebuildBoundingVolumeProcess::update(const ecs::Entity &entity)
{
	auto &boundingVolume = ecs::getOrCreate<BoundingVolume>(entity);

	if ( ! boundingVolume.dirty)
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

	boundingVolume.sphere.position = transform.translation + mesh.boundingSphere.center;
	boundingVolume.sphere.radius = mesh.boundingSphere.radius * max({transform.scale.x, transform.scale.y, transform.scale.z});

	boundingVolume.dirty = false;
}

} // proc
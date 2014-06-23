#include "meshrenderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <app/comp/transform.hpp>
#include <app/comp/material.hpp>
#include <app/comp/mesh.hpp>
#include <core/asset/mesh.hpp>

namespace proc
{

using namespace comp;

void MeshRenderer::render(const ecs::Entity &entity, gl::Program &prog)
{
	if (ecs::has<Material>(entity))
	{
		const auto &material = ecs::get<Material>(entity);
		prog.var("matDiffuse", material.diffuse);
		prog.var("matShininess", material.shininess);
	}

	glm::mat4 M{1.f};

	auto &transform = ecs::get<Transform>(entity);

	M = glm::translate(M, transform.translation);

	M = glm::rotate(M, glm::radians(transform.rotation.x), glm::vec3{1.f, 0.f, 0.f});
	M = glm::rotate(M, glm::radians(transform.rotation.y), glm::vec3{0.f, 1.f, 0.f});
	M = glm::rotate(M, glm::radians(transform.rotation.z), glm::vec3{0.f, 0.f, 1.f});

	M = glm::scale(M, transform.scale);

	prog.var("M", M);

	auto &mesh = asset::mesh::get(ecs::get<Mesh>(entity).id);
	mesh.render();
}

} // proc
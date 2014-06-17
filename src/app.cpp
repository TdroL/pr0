#include "app.hpp"

#include <iostream>
#include <cmath>

#include <core/gl.hpp>
#include <core/ngn.hpp>
#include <core/ngn/key.hpp>
#include <core/ngn/window.hpp>
#include <core/src/sbm.hpp>
#include <core/event.hpp>

#include <core/asset/mesh.hpp>

#include <app/comp/direction.hpp>
#include <app/comp/directionallight.hpp>
#include <app/comp/material.hpp>
#include <app/comp/mesh.hpp>
#include <app/comp/name.hpp>
#include <app/comp/occluder.hpp>
#include <app/comp/pointlight.hpp>
#include <app/comp/position.hpp>
#include <app/comp/projection.hpp>
#include <app/comp/rotation.hpp>
#include <app/comp/transform.hpp>
#include <app/comp/view.hpp>

#include <app/proc/camera.hpp>
#include <app/proc/meshrenderer.hpp>

namespace key = ngn::key;
namespace win = ngn::window;

using namespace comp;
using namespace std;

void App::init()
{
	/* Init scene objects */
	clog << "Init scene objects:" << endl;

	try
	{
		prog.load("lighting/specular.frag", "PN.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		simple.load("color.frag", "PN.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	simple.uniform("color", glm::vec3{1.f});

	try
	{
		fboPreview.load("lighting/fboPreview.frag", "gl/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		deferredGBuffer.load("lighting/deferred/gbuffer.frag", "PN.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		deferredPointLight.load("lighting/deferred/pointlight.frag", "gl/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		deferredDirectionalLight.load("lighting/deferred/directionallight.frag", "gl/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	gbuffer.setTexParams(1, GL_RG16F, GL_RG, GL_HALF_FLOAT);
	gbuffer.setTexParams(2, GL_RGB32F, GL_RGB, GL_FLOAT);

	gbuffer.create({
		"texColor",
		"texNormal",
		"texPosition",
	}, gl::DSType::Texture);

	shadowmap.setDSParams(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);

	shadowmap.create(0, gl::DSType::Texture);

	/* Scene creation */

	// create camera
	cameraId = ecs::create();

	{
		ecs::enable<Name, Position, Rotation, Projection, View>(cameraId);

		auto &name = ecs::get<Name>(cameraId);
		name.name = "Camera";

		auto &position = ecs::get<Position>(cameraId);
		position.x = 0.f;
		position.y = 0.125f;
		position.z = 10.f;

		auto &rotation = ecs::get<Rotation>(cameraId);
		rotation.x = 0.f;
		rotation.y = 0.f;
		rotation.z = 0.f;

		auto &projection = ecs::get<Projection>(cameraId);

		prog.uniform("P", projection.getMatrix());
		simple.uniform("P", projection.getMatrix());
		deferredGBuffer.uniform("P", projection.getMatrix());

		const auto intP = projection.getInverseMatrix();
		deferredPointLight.uniform("invP", intP);
		deferredDirectionalLight.uniform("invP", intP);
	}

	// create models
	ecs::Entity suzanneId = ecs::create();
	ecs::Entity dragonId = ecs::create();
	ecs::Entity venusId = ecs::create();
	ecs::Entity planeId = ecs::create();

	// monkey
	{
		ecs::enable<Name, Transform, Mesh, Material, Occluder>(suzanneId);

		auto &name = ecs::get<Name>(suzanneId);
		name.name = "Monkey";

		auto &transform = ecs::get<Transform>(suzanneId);
		transform.translation = glm::vec3{2.f, 0.f, 0.f};

		auto &material = ecs::get<Material>(suzanneId);
		material.diffuse = glm::vec4{0.2f, 0.8f, 0.8f, 1.f};
		material.shininess = 1.f/2.f;

		ecs::get<Mesh>(suzanneId).id = asset::mesh::load("suzanne.sbm");
	}

	// statue
	{
		ecs::enable<Name, Transform, Mesh, Material, Occluder>(venusId);

		auto &name = ecs::get<Name>(venusId);
		name.name = "Venus";

		auto &transform = ecs::get<Transform>(venusId);
		transform.translation = glm::vec3{-2.f, -0.25f, 0.f};

		auto &material = ecs::get<Material>(venusId);
		material.diffuse = glm::vec4{0.8f, 0.2f, 0.8f, 1.f};
		material.shininess = 1.f/4.f;

		ecs::get<Mesh>(venusId).id = asset::mesh::load("venus.sbm");
	}

	// dragon
	{
		ecs::enable<Name, Transform, Mesh, Material, Occluder>(dragonId);

		auto &name = ecs::get<Name>(dragonId);
		name.name = "Dragon";

		auto &transform = ecs::get<Transform>(dragonId);
		transform.translation = glm::vec3{0.f, -1.5f, -2.f};
		transform.scale = glm::vec3{3.f};

		auto &material = ecs::get<Material>(dragonId);
		material.diffuse = glm::vec4{0.8f, 0.8f, 0.2f, 1.f};
		material.shininess = 1.f/8.f;

		ecs::get<Mesh>(dragonId).id = asset::mesh::load("dragon.sbm");
	}

	// floor
	{
		ecs::enable<Name, Transform, Mesh, Material, Occluder>(planeId);

		auto &name = ecs::get<Name>(planeId);
		name.name = "Plane";

		auto &transform = ecs::get<Transform>(planeId);
		transform.translation = glm::vec3{0.f, -1.5f, 0.f};
		transform.scale = glm::vec3{100.f};

		auto &material = ecs::get<Material>(planeId);
		material.diffuse = glm::vec4{0.2f, 0.8f, 0.2f, 1.f};
		material.shininess = 1.f/16.f;

		ecs::get<Mesh>(planeId).id = asset::mesh::load("plane.sbm");
	}

	// create lights
	lightIds[0] = ecs::create();
	lightIds[1] = ecs::create();
	lightIds[2] = ecs::create();
	lightIds[3] = ecs::create();

	// directional light
	lightIds[4] = ecs::create();

	// light #1
	{
		ecs::enable<Name, PointLight, Position, Transform, Mesh>(lightIds[0]);

		auto &name = ecs::get<Name>(lightIds[0]);
		name.name = "PointLight #1";

		auto &light = ecs::get<PointLight>(lightIds[0]);
		light.color = glm::vec4{1.f, 1.f, 1.f, 1.f};
		// linear attenuation; distance at which half of the light intensity is lost
		light.linearAttenuation = 1.0 / pow(3.0, 2); // r_l = 3.0;
		// quadriatic attenuation; distance at which three-quarters of the light intensity is lost
		light.quadraticAttenuation = 1.0 / pow(6.0, 2); // r_q = 6.0;

		auto &position = ecs::get<Position>(lightIds[0]);
		position.x = 0.f;
		position.y = 1.f;
		position.z = 0.f;

		auto &transform = ecs::get<Transform>(lightIds[0]);
		transform.translation.x = position.x;
		transform.translation.y = position.y;
		transform.translation.z = position.z;
		transform.scale = glm::vec3{0.25f};

		ecs::get<Mesh>(lightIds[0]).id = asset::mesh::load("sphere.sbm");
	}

	// light #2
	{
		ecs::enable<Name, PointLight, Position, Transform, Mesh>(lightIds[1]);

		auto &name = ecs::get<Name>(lightIds[1]);
		name.name = "PointLight #2";

		auto &light = ecs::get<PointLight>(lightIds[1]);
		light.color = glm::vec4{1.f, 1.f, 1.f, 1.f};
		// linear attenuation; distance at which half of the light intensity is lost
		light.linearAttenuation = 1.0 / pow(0.5, 2); // r_l = 3.0;
		// quadriatic attenuation; distance at which three-quarters of the light intensity is lost
		light.quadraticAttenuation = 1.0 / pow(1.0, 2); // r_q = 6.0;

		auto &position = ecs::get<Position>(lightIds[1]);
		position.x = 0.f;
		position.y = 1.f;
		position.z = 0.f;

		auto &transform = ecs::get<Transform>(lightIds[1]);
		transform.translation.x = position.x;
		transform.translation.y = position.y;
		transform.translation.z = position.z;
		transform.scale = glm::vec3{0.25f};

		ecs::get<Mesh>(lightIds[1]).id = asset::mesh::load("sphere.sbm");
	}

	// light #3
	{
		ecs::enable<Name, PointLight, Position, Transform, Mesh>(lightIds[2]);

		auto &name = ecs::get<Name>(lightIds[2]);
		name.name = "PointLight Red";

		auto &light = ecs::get<PointLight>(lightIds[2]);
		light.color = glm::vec4{1.f, 0.f, 0.f, 1.f};
		// linear attenuation; distance at which half of the light intensity is lost
		light.linearAttenuation = 1.0 / pow(1.5, 2); // r_l = 3.0;
		// quadriatic attenuation; distance at which three-quarters of the light intensity is lost
		light.quadraticAttenuation = 1.0 / pow(4.0, 2); // r_q = 6.0;

		auto &position = ecs::get<Position>(lightIds[2]);
		position.x = 2.f;
		position.y = .5f;
		position.z = -4.f;

		auto &transform = ecs::get<Transform>(lightIds[2]);
		transform.translation.x = position.x;
		transform.translation.y = position.y;
		transform.translation.z = position.z;
		transform.scale = glm::vec3{0.25f};

		ecs::get<Mesh>(lightIds[2]).id = asset::mesh::load("sphere.sbm");
	}

	// light #4
	{
		ecs::enable<Name, PointLight, Position, Transform, Mesh>(lightIds[3]);

		auto &name = ecs::get<Name>(lightIds[3]);
		name.name = "PointLight Green";

		auto &light = ecs::get<PointLight>(lightIds[3]);
		light.color = glm::vec4{0.f, 1.f, 0.f, 1.f};
		// linear attenuation; distance at which half of the light intensity is lost
		light.linearAttenuation = 1.0 / pow(1.5, 2); // r_l = 3.0;
		// quadriatic attenuation; distance at which three-quarters of the light intensity is lost
		light.quadraticAttenuation = 1.0 / pow(4.0, 2); // r_q = 6.0;

		auto &position = ecs::get<Position>(lightIds[3]);
		position.x = -3.f;
		position.y = .25f;
		position.z = -.35f;

		auto &transform = ecs::get<Transform>(lightIds[3]);
		transform.translation.x = position.x;
		transform.translation.y = position.y;
		transform.translation.z = position.z;
		transform.scale = glm::vec3{0.25f};

		ecs::get<Mesh>(lightIds[3]).id = asset::mesh::load("sphere.sbm");
	}

	// directional light
	{
		ecs::enable<Name, DirectionalLight, Transform, Mesh>(lightIds[4]);

		auto &name = ecs::get<Name>(lightIds[4]);
		name.name = "DirectionalLight";

		auto &light = ecs::get<DirectionalLight>(lightIds[4]);
		light.direction = glm::vec3{1.f, .25f, 1.f};
		light.color = glm::vec4{.8f, .8f, .8f, 1.f};

		auto &transform = ecs::get<Transform>(lightIds[4]);
		transform.translation = light.direction * 100.f;
		transform.scale = glm::vec3{10.f};

		ecs::get<Mesh>(lightIds[4]).id = asset::mesh::load("sphere.sbm");
	}
}

void App::update()
{
	{
		glm::vec3 translate{0.0};
		glm::vec3 rotate{0.0};

		translate.x = (key::pressed('d') - key::pressed('a'));
		translate.y = (key::pressed(KEY_SPACE) - key::pressed(KEY_CTRL));
		translate.z = (key::pressed('s') - key::pressed('w'));

		if (translate.x != 0.f || translate.z != 0.f)
		{
			glm::vec2 planar = glm::normalize(translate.xz());

			translate.x = planar.x;
			translate.z = planar.y;
		}

		rotate.y = (key::pressed(KEY_LEFT) - key::pressed(KEY_RIGHT));
		rotate.x = (key::pressed(KEY_UP) - key::pressed(KEY_DOWN));

		translate *= 20.0 * ngn::dt;
		rotate *= 90.0 * ngn::dt;

		for (auto &entity : ecs::findWith<Position, Rotation, Projection, View>())
		{
			proc::Camera::update(entity, translate, rotate);
		}
	}

	{
		glm::vec3 lightPositionDelta{static_cast<float>(5.0 * ngn::dt)};

		lightPositionDelta.x *= (ngn::key::pressed(KEY_KP_6) - ngn::key::pressed(KEY_KP_4));
		lightPositionDelta.y *= (ngn::key::pressed(KEY_KP_ADD) - ngn::key::pressed(KEY_KP_ENTER));
		lightPositionDelta.z *= (ngn::key::pressed(KEY_KP_5) - ngn::key::pressed(KEY_KP_8));

		auto &position = ecs::get<Position>(lightIds[0]);
		position.x += lightPositionDelta.x;
		position.y += lightPositionDelta.y;
		position.z += lightPositionDelta.z;

		auto &transform = ecs::get<Transform>(lightIds[0]);
		transform.translation.x = position.x;
		transform.translation.y = position.y;
		transform.translation.z = position.z;
	}

	{
		glm::vec3 lightPositionDelta{3.f, 1.f, 3.f};

		auto &position = ecs::get<Position>(lightIds[1]);
		position.x = lightPositionDelta.x * sin(ngn::ct);
		position.y = lightPositionDelta.y * sin(ngn::ct * 2.f);
		position.z = lightPositionDelta.z * cos(ngn::ct);

		auto &transform = ecs::get<Transform>(lightIds[1]);
		transform.translation.x = position.x;
		transform.translation.y = position.y;
		transform.translation.z = position.z;
	}

	{
		glm::vec3 lightPositionDelta{1.f, .25f, 1.f};

		auto &directionalLight = ecs::get<DirectionalLight>(lightIds[4]);
		directionalLight.direction.x = lightPositionDelta.x * sin(ngn::ct);
		directionalLight.direction.z = lightPositionDelta.z * cos(ngn::ct);

		auto &transform = ecs::get<Transform>(lightIds[4]);
		transform.translation = directionalLight.direction * 100.f;
	}
}

void App::render()
{
	GL_CHECK(glClearColor(0.f, 0.3125f, 1.f, 1.f));
	GL_CHECK(glClearColor(0.f, 0.f, 0.f, 0.f));
	GL_CHECK(glClearDepth(1.f));
	GL_CHECK(glClearStencil(0));
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

	glm::mat4 &V = ecs::get<View>(cameraId).matrix;

	{
		GL_SCOPE_ENABLE(GL_STENCIL_TEST);

		GL_CHECK(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
		GL_CHECK(glStencilMask(0xF));

		GL_SCOPE_DISABLE(GL_BLEND);

		GL_CHECK(glStencilFunc(GL_ALWAYS, 1, 0xF));

		GL_FBO_USE(gbuffer);

		gbuffer.clear();

		deferredGBuffer.use();
		deferredGBuffer.var("V", V);

		for (auto &entity : ecs::findWith<Transform, Mesh, Material>())
		{
			proc::MeshRenderer::render(entity, deferredGBuffer, V);
		}

		deferredGBuffer.release();

		simple.use();
		simple.var("V", V);

		GL_CHECK(glStencilFunc(GL_ALWAYS, 2, 0xF));

		for (auto &entity : ecs::findWith<Transform, Mesh, PointLight>())
		{
			simple.var("color", glm::vec3(ecs::get<PointLight>(entity).color));
			proc::MeshRenderer::render(entity, simple, V);
		}

		for (auto &entity : ecs::findWith<Transform, Mesh, DirectionalLight>())
		{
			simple.var("color", glm::vec3(ecs::get<DirectionalLight>(entity).color));
			proc::MeshRenderer::render(entity, simple, V);
		}

		simple.release();
	}

	{
		gbuffer.blit(0, GL_STENCIL_BUFFER_BIT);

		GL_SCOPE_ENABLE(GL_STENCIL_TEST);

		GL_CHECK(glStencilFunc(GL_EQUAL, 1, 0xF));
		GL_CHECK(glStencilMask(0x0));

		GL_SCOPE_DISABLE(GL_DEPTH_TEST);

		GL_CHECK(glBlendEquation(GL_FUNC_ADD));
		GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));

		for (auto &entity : ecs::findWith<DirectionalLight>())
		{
			auto &light = ecs::get<DirectionalLight>(entity);
			auto lightDirection = V * glm::vec4{light.direction, 0.f};

			{
				GL_FBO_USE(shadowmap);

				shadowmap.clear();

				for (auto &entity : ecs::findWith<Transform, Mesh, Occluder>())
				{
					proc::MeshRenderer::render(entity, deferredGBuffer, V);
				}
			}

			deferredDirectionalLight.var("lightColor", light.color);
			deferredDirectionalLight.var("lightDirection", lightDirection.xyz());

			gbuffer.render(deferredDirectionalLight);
		}

		for (auto &entity : ecs::findWith<Transform, Mesh, PointLight>())
		{
			auto &light = ecs::get<PointLight>(entity);
			auto &lightTransform = ecs::get<Transform>(entity);
			auto lightPosition = V * glm::vec4{lightTransform.translation, 1.f};

			if (false)
			{
				GL_FBO_USE(shadowmap);

				shadowmap.clear();

				for (auto &entity : ecs::findWith<Transform, Mesh, Occluder>())
				{
					proc::MeshRenderer::render(entity, deferredGBuffer, V);
				}
			}

			deferredPointLight.var("lightPosition", lightPosition);
			deferredPointLight.var("lightColor", light.color);
			deferredPointLight.var("lightLinearAttenuation", light.linearAttenuation);
			deferredPointLight.var("lightQuadraticAttenuation", light.quadraticAttenuation);

			gbuffer.render(deferredPointLight);
		}

		GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		GL_CHECK(glStencilFunc(GL_EQUAL, 2, 0xFF));

		gbuffer.render(fboPreview);
	}
}
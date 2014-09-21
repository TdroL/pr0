#include "app.hpp"

#include <iostream>
#include <cmath>
#include <limits>

#include <core/gl.hpp>
#include <core/ngn.hpp>
#include <core/ngn/key.hpp>
#include <core/ngn/window.hpp>
#include <core/src/sbm.hpp>
#include <core/event.hpp>
#include <core/util/timer.hpp>

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
#include <app/comp/stencil.hpp>
#include <app/comp/transform.hpp>
#include <app/comp/view.hpp>

#include <app/proc/camera.hpp>
#include <app/proc/meshrenderer.hpp>

namespace key = ngn::key;
namespace win = ngn::window;

using namespace comp;
using namespace std;

util::Timer sunTimer{};

enum Shader {
	global = 1,
	skin = 2,
	flat = 3,
	MASK = 1 | 2 | 3,
};

void App::init()
{
	/* Init scene objects */
	clog << "Init scene objects:" << endl;

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
		blurPreview.load("gl/blurPreview.frag", "gl/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		shadowMapPreview.load("lighting/shadows/preview.frag", "gl/fboM.vert");
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
		blurFilter.load("gl/blur.frag", "gl/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		deferredShadowMap.load("lighting/shadows/depthVSM.frag", "P.vert");
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

	try
	{
		deferredSkinDirectionalLight.load("lighting/deferred/skindirectionallight.frag", "gl/fbo.vert");
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

	gBuffer.setTex(0, gl::Tex2D{GL_RGBA16F, GL_RGBA, GL_FLOAT});
	gBuffer.setTex(1, gl::Tex2D{GL_RG16F, GL_RG, GL_FLOAT});
	gBuffer.setDepth(gl::Tex2D{GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8});
	gBuffer.create();

	shadowMapBuffer.width = 1024;
	shadowMapBuffer.height = 1024;
	shadowMapBuffer.clearColor = glm::vec4{numeric_limits<GLfloat>::max()};
	shadowMapBuffer.setTex(0, gl::Tex2D{GL_RGB32F, GL_RGB, GL_FLOAT});
	shadowMapBuffer.setDepth(gl::Tex2D{GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT});
	shadowMapBuffer.create();

	blurBuffer.width = shadowMapBuffer.width;
	blurBuffer.height = shadowMapBuffer.height;
	blurBuffer.clearColor = glm::vec4{numeric_limits<GLfloat>::max()};
	blurBuffer.setTex(0, gl::Tex2D{GL_RGB32F, GL_RGB, GL_FLOAT});
	blurBuffer.setDepth(gl::Renderbuffer{GL_DEPTH_COMPONENT32F});
	blurBuffer.create();

	/* Scene creation */

	// create camera
	cameraId = ecs::create();

	{
		ecs::enable<Name, Position, Rotation, Projection, View>(cameraId);

		auto &name = ecs::get<Name>(cameraId);
		name.name = "Camera";

		auto &position = ecs::get<Position>(cameraId).position;
		position.x = 0.f;
		position.y = 0.125f;
		position.z = 10.f;

		auto &rotation = ecs::get<Rotation>(cameraId).rotation;
		rotation.x = 0.f;
		rotation.y = 0.f;
		rotation.z = 0.f;

		auto &projection = ecs::get<Projection>(cameraId);

		simple.uniform("P", projection.getMatrix());
		deferredGBuffer.uniform("P", projection.getMatrix());

		const auto intP = glm::inverse(projection.getMatrix());
		deferredPointLight.uniform("invP", intP);
		deferredDirectionalLight.uniform("invP", intP);
		deferredSkinDirectionalLight.uniform("invP", intP);

		float winRatio = static_cast<float>(win::width) / static_cast<float>(win::height);
		glm::mat4 previewM{1.f};
		previewM = glm::translate(previewM, glm::vec3{.75f, .75f, 0.f});
		previewM = glm::scale(previewM, glm::vec3{.25f / winRatio, .25f, 0.f});
		// previewM = glm::scale(previewM, glm::vec3{.25f});

		shadowMapPreview.uniform("M", previewM);
		blurPreview.uniform("M", previewM);
	}

	// create models
	ecs::Entity suzanneId = ecs::create();
	ecs::Entity dragonId = ecs::create();
	ecs::Entity venusId = ecs::create();
	ecs::Entity planeId = ecs::create();

	// monkey
	{
		ecs::enable<Name, Transform, Mesh, Material, Occluder, Stencil>(suzanneId);

		auto &name = ecs::get<Name>(suzanneId);
		name.name = "Monkey";

		auto &transform = ecs::get<Transform>(suzanneId);
		transform.translation = glm::vec3{2.f, 0.f, 0.f};

		auto &material = ecs::get<Material>(suzanneId);
		material.diffuse = glm::vec4{248.f/255.f, 185.f/255.f, 142.f/255.f, 1.f};
		material.shininess = 1.f/2.f;

		auto &stencil = ecs::get<Stencil>(suzanneId);
		stencil.ref = Shader::skin;

		ecs::get<Mesh>(suzanneId).id = asset::mesh::load("suzanne.sbm");
	}

	// statue
	{
		ecs::enable<Name, Transform, Mesh, Material, Occluder, Stencil>(venusId);

		auto &name = ecs::get<Name>(venusId);
		name.name = "Venus";

		auto &transform = ecs::get<Transform>(venusId);
		transform.translation = glm::vec3{-2.f, -0.25f, 0.f};

		auto &material = ecs::get<Material>(venusId);
		material.diffuse = glm::vec4{0.8f, 0.2f, 0.8f, 1.f};
		material.shininess = 1.f/4.f;

		auto &stencil = ecs::get<Stencil>(venusId);
		stencil.ref = Shader::global; // global, skin, flat

		ecs::get<Mesh>(venusId).id = asset::mesh::load("venus.sbm");
	}

	// dragon
	{
		ecs::enable<Name, Transform, Mesh, Material, Occluder, Stencil>(dragonId);

		auto &name = ecs::get<Name>(dragonId);
		name.name = "Dragon";

		auto &transform = ecs::get<Transform>(dragonId);
		transform.translation = glm::vec3{0.f, -1.5f, -2.f};
		transform.scale = glm::vec3{3.f};

		auto &material = ecs::get<Material>(dragonId);
		material.diffuse = glm::vec4{0.8f, 0.8f, 0.2f, 1.f};
		material.shininess = 1.f/8.f;

		auto &stencil = ecs::get<Stencil>(dragonId);
		stencil.ref = Shader::global; // global, skin, flat

		ecs::get<Mesh>(dragonId).id = asset::mesh::load("dragon.sbm");
	}

	// floor
	{
		ecs::enable<Name, Transform, Mesh, Material, Occluder, Stencil>(planeId);

		auto &name = ecs::get<Name>(planeId);
		name.name = "Plane";

		auto &transform = ecs::get<Transform>(planeId);
		transform.translation = glm::vec3{0.f, -1.5f, 0.f};
		transform.scale = glm::vec3{100.f};

		auto &material = ecs::get<Material>(planeId);
		material.diffuse = glm::vec4{0.2f, 0.8f, 0.2f, 1.f};
		material.shininess = 1.f/16.f;

		auto &stencil = ecs::get<Stencil>(planeId);
		stencil.ref = Shader::global; // global, skin, flat

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

		auto &position = ecs::get<Position>(lightIds[0]).position;
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

		auto &position = ecs::get<Position>(lightIds[1]).position;
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

		auto &position = ecs::get<Position>(lightIds[2]).position;
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

		auto &position = ecs::get<Position>(lightIds[3]).position;
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

	// directional light #1
	{
		ecs::enable<Name, DirectionalLight, Transform, Mesh>(lightIds[4]);

		auto &name = ecs::get<Name>(lightIds[4]);
		name.name = "DirectionalLight";

		auto &light = ecs::get<DirectionalLight>(lightIds[4]);
		light.direction = glm::vec3{.5f, .25f, 1.f};
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

		proc::Camera::update(cameraId, translate, rotate);
	}

	if (key::hit('t'))
	{
		sunTimer.togglePause();
	}

	{
		glm::vec3 lightPositionDelta{static_cast<float>(5.0 * ngn::dt)};

		lightPositionDelta.x *= (ngn::key::pressed(KEY_KP_6)   - ngn::key::pressed(KEY_KP_4));
		lightPositionDelta.y *= (ngn::key::pressed(KEY_KP_ADD) - ngn::key::pressed(KEY_KP_ENTER));
		lightPositionDelta.z *= (ngn::key::pressed(KEY_KP_5)   - ngn::key::pressed(KEY_KP_8));

		auto &position = ecs::get<Position>(lightIds[0]).position;
		position += lightPositionDelta;

		auto &transform = ecs::get<Transform>(lightIds[0]);
		transform.translation = position;
	}

	{
		glm::vec3 lightPositionDelta{3.f, 1.f, 3.f};

		auto &position = ecs::get<Position>(lightIds[1]).position;
		position.x = lightPositionDelta.x * sin(ngn::ct * .5f);
		position.y = lightPositionDelta.y * sin(ngn::ct);
		position.z = lightPositionDelta.z * cos(ngn::ct * .5f);

		auto &transform = ecs::get<Transform>(lightIds[1]);
		transform.translation = position;
	}

	{
		glm::vec3 lightPositionDelta{1.f, .25f, 1.f};

		auto &directionalLight = ecs::get<DirectionalLight>(lightIds[4]);
		directionalLight.direction.x = lightPositionDelta.x * sin(sunTimer.timed);
		directionalLight.direction.z = lightPositionDelta.z * cos(sunTimer.timed);

		auto &transform = ecs::get<Transform>(lightIds[4]);
		const auto &camPosition = ecs::get<Position>(cameraId).position;
		transform.translation = camPosition + directionalLight.direction * 100.f;
	}

	sunTimer.update();
}

void App::render()
{
	GL_CHECK(glClearColor(0.f, 0.f, 0.f, 0.f));
	GL_CHECK(glClearDepth(numeric_limits<GLfloat>::max()));
	GL_CHECK(glClearStencil(0));
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

	gBufferPass();

	gBuffer.blit(0, GL_STENCIL_BUFFER_BIT);

	directionalLightsPass();
	pointLightsPass();
	flatLightPass();

	{
		shadowMapPreview.use();

		shadowMapBuffer.colors[0].bind(0);
		shadowMapPreview.var("texSource", 0);

		gl::FBO::mesh.render();
	}
}

void App::gBufferPass()
{
	GL_SCOPE_ENABLE(GL_STENCIL_TEST);

	GL_CHECK(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
	GL_CHECK(glStencilMask(0xF));

	GL_SCOPE_DISABLE(GL_BLEND);

	GL_FBO_USE(gBuffer);
	gBuffer.clear();

	glm::mat4 &V = ecs::get<View>(cameraId).matrix;

	deferredGBuffer.use();
	deferredGBuffer.var("V", V);

	for (auto &entity : ecs::findWith<Transform, Mesh, Material>())
	{
		GLint ref = ecs::has<Stencil>(entity) ? ecs::get<Stencil>(entity).ref : 0;

		GL_CHECK(glStencilFunc(GL_ALWAYS, ref, 0xF));

		proc::MeshRenderer::render(entity, deferredGBuffer);
	}

	// draw light meshes
	simple.use();
	simple.var("V", V);

	GL_CHECK(glStencilFunc(GL_ALWAYS, Shader::flat, 0xF));

	for (auto &entity : ecs::findWith<Transform, Mesh, PointLight>())
	{
		simple.var("color", glm::vec3(ecs::get<PointLight>(entity).color));
		proc::MeshRenderer::render(entity, simple);
	}

	for (auto &entity : ecs::findWith<Transform, Mesh, DirectionalLight>())
	{
		simple.var("color", glm::vec3(ecs::get<DirectionalLight>(entity).color));
		proc::MeshRenderer::render(entity, simple);
	}
}

void App::directionalLightsPass()
{
	glm::mat4 &V = ecs::get<View>(cameraId).matrix;

	deferredDirectionalLight.var("invV", glm::inverse(V));
	deferredSkinDirectionalLight.var("invV", glm::inverse(V));

	for (auto &entity : ecs::findWith<DirectionalLight>())
	{
		glm::mat4 shadowMapMVP = genShadowMap(entity);

		const auto &light = ecs::get<DirectionalLight>(entity);

		GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));
		GL_SCOPE_ENABLE(GL_STENCIL_TEST);
		GL_CHECK(glStencilMask(0x0));

		gBuffer.colors[0].bind(0);
		gBuffer.colors[1].bind(1);
		gBuffer.depth.tex.bind(2);
		shadowMapBuffer.colors[0].bind(3);
		shadowMapBuffer.depth.tex.bind(4);

		// global lighting
		{
			GL_CHECK(glStencilFunc(GL_EQUAL, Shader::global, Shader::MASK));

			deferredDirectionalLight.use();

			deferredDirectionalLight.var("lightColor", light.color);
			deferredDirectionalLight.var("lightDirection", glm::mat3{V} * light.direction);
			deferredDirectionalLight.var("shadowmapMVP", shadowMapMVP);

			deferredDirectionalLight.var("texColor", 0);
			deferredDirectionalLight.var("texNormal", 1);
			deferredDirectionalLight.var("texDepth", 2);
			deferredDirectionalLight.var("shadowMoments", 3);
			deferredDirectionalLight.var("shadowDepth", 4);

			gl::FBO::mesh.render();
		}

		// skin lighting
		{
			GL_CHECK(glStencilFunc(GL_EQUAL, Shader::skin, Shader::MASK));

			deferredSkinDirectionalLight.use();

			deferredSkinDirectionalLight.var("lightColor", light.color);
			deferredSkinDirectionalLight.var("lightDirection", glm::mat3{V} * light.direction);
			deferredSkinDirectionalLight.var("shadowmapMVP", shadowMapMVP);

			deferredSkinDirectionalLight.var("texColor", 0);
			deferredSkinDirectionalLight.var("texNormal", 1);
			deferredSkinDirectionalLight.var("texDepth", 2);
			deferredSkinDirectionalLight.var("shadowMoments", 3);
			deferredSkinDirectionalLight.var("shadowDepth", 4);

			gl::FBO::mesh.render();
		}

		GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}
}

void App::pointLightsPass()
{
	glm::mat4 &V = ecs::get<View>(cameraId).matrix;

	for (auto &entity : ecs::findWith<Transform, Mesh, PointLight>())
	{
		const auto &light = ecs::get<PointLight>(entity);
		const auto &lightTransform = ecs::get<Transform>(entity);
		const auto lightPosition = V * glm::vec4{lightTransform.translation, 1.f};

		// @TODO: shadows
		// {
		// 	GL_FBO_USE(shadowmap);

		// 	shadowmap.clear();

		// 	deferredShadowMap.use();
		// 	deferredShadowMap.var("V", V);

		// 	for (auto &entity : ecs::findWith<Transform, Mesh, Occluder>())
		// 	{
		// 		proc::MeshRenderer::render(entity, deferredShadowMap);
		// 	}
		// }

		{
			GL_SCOPE_ENABLE(GL_STENCIL_TEST);

			GL_CHECK(glStencilFunc(GL_EQUAL, Shader::global, Shader::MASK));
			GL_CHECK(glStencilMask(0x0));

			GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));

			deferredPointLight.use();
			deferredPointLight.var("lightPosition", lightPosition);
			deferredPointLight.var("lightColor", light.color);
			deferredPointLight.var("lightLinearAttenuation", light.linearAttenuation);
			deferredPointLight.var("lightQuadraticAttenuation", light.quadraticAttenuation);

			gBuffer.colors[0].bind(0);
			gBuffer.colors[1].bind(1);
			gBuffer.depth.tex.bind(2);
			// shadowMapBuffer.colors[0].bind(3);
			// shadowMapBuffer.depth.tex.bind(4);

			deferredPointLight.var("texColor", 0);
			deferredPointLight.var("texNormal", 1);
			deferredPointLight.var("texDepth", 2);
			// deferredPointLight.var("shadowMoments", 3);
			// deferredPointLight.var("shadowDepth", 4);

			gl::FBO::mesh.render();

			GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		}
	}
}

void App::flatLightPass()
{
	GL_SCOPE_ENABLE(GL_STENCIL_TEST);

	GL_CHECK(glStencilFunc(GL_EQUAL, Shader::flat, Shader::MASK));
	GL_CHECK(glStencilMask(0x0));

	gl::FBO::prog.use();

	gBuffer.colors[0].bind(0);

	gl::FBO::prog.var("tex0", 0);

	gl::FBO::mesh.render();
}

glm::mat4 App::genShadowMap(ecs::Entity lightId)
{
	const auto &light = ecs::get<DirectionalLight>(lightId);

	glm::mat4 shadowMapP = glm::ortho(-10.f, 10.f, -10.f, 10.f, -10.f, 20.f);
	glm::mat4 shadowMapV = glm::lookAt(glm::normalize(light.direction), glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.f, 1.f, 0.f});
	glm::mat4 shadowMapM{1.f};
	glm::mat4 shadowMapMVP = shadowMapP * shadowMapV * shadowMapM;

	{
		GL_FBO_USE(shadowMapBuffer);

		GL_SCOPE_DISABLE(GL_BLEND);

		// GL_CHECK(glCullFace(GL_FRONT));

		shadowMapBuffer.clear();

		deferredShadowMap.use();
		deferredShadowMap.var("P", shadowMapP);
		deferredShadowMap.var("V", shadowMapV);

		for (auto &entity : ecs::findWith<Transform, Mesh, Occluder>())
		{
			proc::MeshRenderer::render(entity, deferredShadowMap);
		}

		// GL_CHECK(glCullFace(GL_BACK));
	}

	// blur shadowmap: x pass
	{
		GL_FBO_USE(blurBuffer);

		GL_SCOPE_DISABLE(GL_DEPTH_TEST);

		blurFilter.use();

		shadowMapBuffer.colors[0].bind(0);

		blurFilter.var("texSource", 0);
		blurFilter.var("scale", glm::vec2{1.f / blurBuffer.width, 0.0f /* blurBuffer.height */});

		gl::FBO::mesh.render();
	}

	// blur shadowmap: y pass
	{
		GL_FBO_USE(shadowMapBuffer);

		GL_SCOPE_DISABLE(GL_DEPTH_TEST);

		blurFilter.use();

		blurBuffer.colors[0].bind(0);

		blurFilter.var("texSource", 0);
		blurFilter.var("scale", glm::vec2{0.f /* blurBuffer.width */, 1.0f / blurBuffer.height});

		gl::FBO::mesh.render();
	}

	return shadowMapMVP;
}
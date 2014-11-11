#include "app.hpp"

#include <iostream>
#include <cmath>
#include <limits>

#include <glm/gtc/random.hpp>
#include <glm/gtx/compatibility.hpp>

#include <core/rn.hpp>
#include <core/ngn.hpp>
#include <core/ngn/key.hpp>
#include <core/ngn/window.hpp>
#include <core/src/sbm.hpp>
#include <core/src/mem.hpp>
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
	flat = 2,
	MASK = 1 | 2,
};

void App::init()
{
	/* Init scene objects */
	clog << "Init scene objects:" << endl;

	try
	{
		progFBOBlit.load("rn/fboBlit.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	progFBOBlit.uniform("color", glm::vec3{1.f});

	try
	{
		progBlurPreview.load("rn/blurPreview.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progShadowMapPreview.load("lighting/shadows/preview.frag", "rn/fboM.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progGBuffer.load("lighting/deferred/gbuffer.frag", "PN.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progBlurGaussian7.load("rn/blurGaussian7.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progShadowMap.load("lighting/shadows/depthVSM.frag", "P.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progDirectionalLight.load("lighting/deferred/directionallight.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progPointLight.load("lighting/deferred/pointlight.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progFlatLight.load("lighting/deferred/flatlight.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progSSAO.load("lighting/ssao.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progSSAOBlit.load("lighting/ssaoBlit.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progSSAOBlur.load("lighting/ssaoBlur.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	fboGBuffer.width = win::width;
	fboGBuffer.height = win::height;
	fboGBuffer.setColorTex(GL_RGBA16F, 0);
	fboGBuffer.setColorTex(GL_RG16F, 1);
	fboGBuffer.setDepthTex(GL_DEPTH24_STENCIL8);
	fboGBuffer.create();

	fboShadowMapBuffer.width = 2048;
	fboShadowMapBuffer.height = 2048;
	fboShadowMapBuffer.clearColor = glm::vec4{numeric_limits<GLfloat>::max()};
	fboShadowMapBuffer.setColorTex(GL_RGB32F, 0);
	fboShadowMapBuffer.setDepthTex(GL_DEPTH_COMPONENT32F);
	fboShadowMapBuffer.create();

	fboShadowMapBlurBuffer.clone(fboShadowMapBuffer);
	fboShadowMapBlurBuffer.setDepthTex(GL_NONE);
	fboShadowMapBlurBuffer.create();

	fboSSAOBuffer.width = win::width;
	fboSSAOBuffer.height = win::height;
	// fboSSAOBuffer.setColorTex(GL_R8, 0);
	fboSSAOBuffer.setColorTex(GL_RGBA8, 0);
	fboSSAOBuffer.setDepthBuf(GL_DEPTH24_STENCIL8);
	fboSSAOBuffer.create();

	fboSSAOBlurBuffer.clone(fboSSAOBuffer);
	fboSSAOBlurBuffer.create();

	// gen SSAO kernel
	size_t kernelSize = 64;
	unique_ptr<glm::vec3[]> kernel{new glm::vec3[kernelSize]};

	float minCosine = glm::cos(glm::radians(90.f * 0.25f));

	for (size_t i = 0; i < kernelSize; i++)
	{
		glm::vec3 b;
		float cosine = 0.f;

		do
		{
			kernel[i] = glm::sphericalRand(1.f);
			kernel[i].z = abs(kernel[i].z);

			b = glm::normalize(glm::vec3{kernel[i].x, kernel[i].y, 0.f});
			cosine = glm::dot(kernel[i], b);
		}
		while (cosine > minCosine);
	}

	// gen SSAO noise texture
	int noiseWidth = 4;
	int noiseHeight = 4;
	size_t noiseSize = noiseWidth * noiseHeight * sizeof(GLuint);
	unique_ptr<GLubyte[]> noise{new GLubyte[noiseSize]};

	float formatScale = (1 << (8 * sizeof(GLubyte))) - 1; // 255

	for (size_t i = 0; i < noiseSize; i += sizeof(GLuint))
	{
		glm::ivec2 vec = glm::ivec2{glm::circularRand(1.0f) * formatScale};

		noise[i + 3] = static_cast<GLubyte>(vec.x); // x - red
		noise[i + 2] = static_cast<GLubyte>(vec.y); // y - green
		noise[i + 1] = 0; // z - blue
		noise[i + 0] = 0; // w - alpha
	}

	texNoise.source = src::mem::tex2d(4, 4, move(noise), noiseSize, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8);
	texNoise.reload();

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

		const auto &P = projection.getMatrix();
		progGBuffer.uniform("P", P);
		progSSAO.uniform("P", P);

		const auto invP = glm::inverse(P);
		progPointLight.uniform("invP", invP);
		progDirectionalLight.uniform("invP", invP);
		progSSAO.uniform("invP", invP);
		progSSAOBlur.uniform("invP", invP);

		float winRatio = static_cast<float>(win::width) / static_cast<float>(win::height);
		glm::mat4 previewM{1.f};
		previewM = glm::translate(previewM, glm::vec3{.75f, .75f, 0.f});
		previewM = glm::scale(previewM, glm::vec3{.25f / winRatio, .25f, 0.f});
		// previewM = glm::scale(previewM, glm::vec3{.25f});

		progShadowMapPreview.uniform("M", previewM);
		progBlurPreview.uniform("M", previewM);

		progSSAO.uniform("kernel", move(kernel), kernelSize);
		progSSAO.uniform("noiseScale", glm::vec2{win::width / noiseWidth, win::height / noiseHeight});
	}

	// create models

	// monkey
	{
		ecs::Entity suzanneId = ecs::create();
		ecs::enable<Name, Transform, Mesh, Material, Occluder, Stencil>(suzanneId);

		auto &name = ecs::get<Name>(suzanneId);
		name.name = "Monkey";

		auto &transform = ecs::get<Transform>(suzanneId);
		transform.translation = glm::vec3{2.f, 0.f, 0.f};

		auto &material = ecs::get<Material>(suzanneId);
		material.diffuse = glm::vec4{248.f/255.f, 185.f/255.f, 142.f/255.f, 1.f};
		material.shininess = 1.f/2.f;

		auto &stencil = ecs::get<Stencil>(suzanneId);
		stencil.ref = Shader::global;

		ecs::get<Mesh>(suzanneId).id = asset::mesh::load("suzanne.sbm");
	}

	// statue
	{
		ecs::Entity venusId = ecs::create();
		ecs::enable<Name, Transform, Mesh, Material, Occluder, Stencil>(venusId);

		auto &name = ecs::get<Name>(venusId);
		name.name = "Venus";

		auto &transform = ecs::get<Transform>(venusId);
		transform.translation = glm::vec3{-2.f, -0.25f, 0.f};

		auto &material = ecs::get<Material>(venusId);
		material.diffuse = glm::vec4{0.8f, 0.2f, 0.8f, 1.f};
		material.shininess = 1.f/4.f;

		auto &stencil = ecs::get<Stencil>(venusId);
		stencil.ref = Shader::global;

		ecs::get<Mesh>(venusId).id = asset::mesh::load("venus.sbm");
	}

	// dragon
	{
		ecs::Entity dragonId = ecs::create();
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
		stencil.ref = Shader::global;

		ecs::get<Mesh>(dragonId).id = asset::mesh::load("dragon.sbm");
	}

	// floor
	{
		ecs::Entity planeId = ecs::create();
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
		stencil.ref = Shader::global;

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
	RN_CHECK(glClearColor(0.f, 0.f, 0.f, 0.f));
	RN_CHECK(glClearDepth(numeric_limits<GLfloat>::max()));
	RN_CHECK(glClearStencil(0));
	RN_CHECK(glStencilMask(0xF));
	RN_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

	gBufferPass();

	fboGBuffer.blit(0, GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	directionalLightsPass();
	// pointLightsPass();
	flatLightPass();

	ssao();

	{
		progShadowMapPreview.use();

		progShadowMapPreview.var("texSource", fboShadowMapBuffer.bindColorTex(0, 0));

		rn::FBO::mesh.render();

		progShadowMapPreview.forgo();
	}
}

void App::gBufferPass()
{
	RN_SCOPE_DISABLE(GL_BLEND);
	RN_SCOPE_ENABLE(GL_STENCIL_TEST);

	RN_CHECK(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
	RN_CHECK(glStencilMask(0xF));

	RN_FBO_USE(fboGBuffer);
	fboGBuffer.clear();

	glm::mat4 &V = ecs::get<View>(cameraId).matrix;

	progGBuffer.use();
	progGBuffer.var("V", V);

	for (auto &entity : ecs::findWith<Transform, Mesh, Material>())
	{
		GLint ref = ecs::has<Stencil>(entity) ? ecs::get<Stencil>(entity).ref : 0;

		RN_CHECK(glStencilFunc(GL_ALWAYS, ref, 0xF));

		proc::MeshRenderer::render(entity, progGBuffer);
	}

	// draw light meshes
	RN_CHECK(glStencilFunc(GL_ALWAYS, Shader::flat, 0xF));

	progGBuffer.var("matShininess", 0.f);

	for (auto &entity : ecs::findWith<Transform, Mesh, PointLight>())
	{
		progGBuffer.var("matDiffuse", ecs::get<PointLight>(entity).color);
		proc::MeshRenderer::render(entity, progGBuffer);
	}

	for (auto &entity : ecs::findWith<Transform, Mesh, DirectionalLight>())
	{
		progGBuffer.var("matDiffuse", ecs::get<DirectionalLight>(entity).color);
		proc::MeshRenderer::render(entity, progGBuffer);
	}

	progGBuffer.forgo();
}

void App::directionalLightsPass()
{
	glm::mat4 &V = ecs::get<View>(cameraId).matrix;
	glm::mat4 invV = glm::inverse(V);

	progDirectionalLight.var("invV", invV);

	for (auto &entity : ecs::findWith<DirectionalLight>())
	{
		glm::mat4 shadowMapMVP = genShadowMap(entity);

		const auto &light = ecs::get<DirectionalLight>(entity);

		RN_CHECK(glBlendFunc(GL_ONE, GL_ONE));
		RN_SCOPE_ENABLE(GL_STENCIL_TEST);
		RN_CHECK(glStencilFunc(GL_EQUAL, Shader::global, Shader::MASK));
		RN_CHECK(glStencilMask(0x0));

		// global lighting
		{

			progDirectionalLight.use();

			progDirectionalLight.var("lightColor", light.color);
			progDirectionalLight.var("lightDirection", glm::mat3{V} * light.direction);
			progDirectionalLight.var("shadowmapMVP", shadowMapMVP);

			progDirectionalLight.var("texColor", fboGBuffer.bindColorTex(0, 0));
			progDirectionalLight.var("texNormal", fboGBuffer.bindColorTex(1, 1));
			progDirectionalLight.var("texDepth", fboGBuffer.bindDepthTex(2));

			progDirectionalLight.var("shadowMoments", fboShadowMapBuffer.bindColorTex(3, 0));
			progDirectionalLight.var("shadowDepth", fboShadowMapBuffer.bindDepthTex(4));

			rn::FBO::mesh.render();

			progDirectionalLight.forgo();
		}

		RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
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
		// 	RN_FBO_USE(shadowmap);

		// 	shadowmap.clear();

		// 	progShadowMap.use();
		// 	progShadowMap.var("V", V);

		// 	for (auto &entity : ecs::findWith<Transform, Mesh, Occluder>())
		// 	{
		// 		proc::MeshRenderer::render(entity, progShadowMap);
		// 	}

		// 	progShadowMap.forgo();
		// }

		{
			RN_SCOPE_ENABLE(GL_STENCIL_TEST);
			RN_CHECK(glStencilFunc(GL_EQUAL, Shader::global, Shader::MASK));
			RN_CHECK(glStencilMask(0x0));

			RN_CHECK(glBlendFunc(GL_ONE, GL_ONE));

			progPointLight.use();
			progPointLight.var("lightPosition", lightPosition);
			progPointLight.var("lightColor", light.color);
			progPointLight.var("lightLinearAttenuation", light.linearAttenuation);
			progPointLight.var("lightQuadraticAttenuation", light.quadraticAttenuation);

			progPointLight.var("texColor", fboGBuffer.bindColorTex(0, 0));
			progPointLight.var("texNormal", fboGBuffer.bindColorTex(1, 1));
			progPointLight.var("texDepth", fboGBuffer.bindDepthTex(2));

			// shadowMapBuffer.colors[0].bind(3);
			// shadowMapBuffer.depth.tex.bind(4);

			// progPointLight.var("shadowMoments", 3);
			// progPointLight.var("shadowDepth", 4);

			rn::FBO::mesh.render();

			progPointLight.forgo();

			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		}
	}
}

void App::flatLightPass()
{
	RN_SCOPE_ENABLE(GL_STENCIL_TEST);
	RN_CHECK(glStencilFunc(GL_EQUAL, Shader::flat, Shader::MASK));
	RN_CHECK(glStencilMask(0x0));

	progFlatLight.use();

	progFlatLight.var("texColor", fboGBuffer.bindColorTex(0, 0));

	rn::FBO::mesh.render();

	progFlatLight.forgo();
}

void App::ssao()
{
	RN_SCOPE_DISABLE(GL_DEPTH_TEST);

	{
		RN_SCOPE_DISABLE(GL_BLEND);

		RN_FBO_USE(fboSSAOBuffer);
		fboSSAOBuffer.clear(GL_COLOR_BUFFER_BIT);

		progSSAO.use();

		progSSAO.var("texColor", fboGBuffer.bindColorTex(0, 0));
		progSSAO.var("texNormal", fboGBuffer.bindColorTex(1, 1));
		progSSAO.var("texDepth", fboGBuffer.bindDepthTex(2));
		progSSAO.var("texNoise", texNoise.bind(3));

		progSSAO.var("filterRadius", glm::vec2{10.f / win::width, 10.f / win::height }); // 10px filter radius

		rn::FBO::mesh.render();

		progSSAO.forgo();
	}

	// blur: x pass
	{
		RN_SCOPE_DISABLE(GL_BLEND);

		RN_FBO_USE(fboSSAOBlurBuffer);
		fboSSAOBlurBuffer.clear(GL_COLOR_BUFFER_BIT);

		progSSAOBlur.use();

		progSSAOBlur.var("texSource", fboSSAOBuffer.bindColorTex(0, 0));
		progSSAOBlur.var("texNormal", fboGBuffer.bindColorTex(1, 1));
		progSSAOBlur.var("texDepth", fboGBuffer.bindDepthTex(2));
		progSSAOBlur.var("scale", glm::vec2{1.f, 0.f});

		rn::FBO::mesh.render();

		progSSAOBlur.forgo();
	}

	// blur: y pass
	{
		RN_SCOPE_DISABLE(GL_BLEND);

		RN_FBO_USE(fboSSAOBuffer);
		fboSSAOBuffer.clear(GL_COLOR_BUFFER_BIT);

		progSSAOBlur.use();

		progSSAOBlur.var("texSource", fboSSAOBlurBuffer.bindColorTex(0, 0));
		progSSAOBlur.var("texNormal", fboGBuffer.bindColorTex(1, 1));
		progSSAOBlur.var("texDepth", fboGBuffer.bindDepthTex(2));
		progSSAOBlur.var("scale", glm::vec2{0.f, 1.f});

		rn::FBO::mesh.render();

		progSSAOBlur.forgo();
	}

	if (!true)
	{
		RN_SCOPE_DISABLE(GL_BLEND);
		progFBOBlit.use();

		progFBOBlit.var("texSource", fboSSAOBuffer.bindColorTex(0, 0));

		rn::FBO::mesh.render();

		progFBOBlit.forgo();

		return;
	}

	{
		RN_SCOPE_ENABLE(GL_STENCIL_TEST);
		RN_CHECK(glStencilFunc(GL_EQUAL, Shader::global, Shader::MASK));
		RN_CHECK(glStencilMask(0x0));

		progSSAOBlit.use();

		progSSAOBlit.var("texSource", fboSSAOBuffer.bindColorTex(0, 0));
		progSSAOBlit.var("texColor", fboGBuffer.bindColorTex(1, 0));
		progSSAOBlit.var("texNormal", fboGBuffer.bindColorTex(2, 1));
		progSSAOBlit.var("texDepth", fboGBuffer.bindDepthTex(3));

		rn::FBO::mesh.render();

		progSSAOBlit.forgo();
	}
}

glm::mat4 App::genShadowMap(ecs::Entity lightId)
{
	const auto &light = ecs::get<DirectionalLight>(lightId);

	glm::mat4 shadowMapP = glm::ortho(-25.f, 25.f, -25.f, 25.f, -25.f, 50.f);
	glm::mat4 shadowMapV = glm::lookAt(glm::normalize(light.direction), glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.f, 1.f, 0.f});
	glm::mat4 shadowMapM{1.f};
	glm::mat4 shadowMapMVP = shadowMapP * shadowMapV * shadowMapM;

	{
		RN_FBO_USE(fboShadowMapBuffer);

		RN_SCOPE_DISABLE(GL_BLEND);

		// RN_CHECK(glCullFace(GL_FRONT));

		fboShadowMapBuffer.clear();

		progShadowMap.use();
		progShadowMap.var("P", shadowMapP);
		progShadowMap.var("V", shadowMapV);

		for (auto &entity : ecs::findWith<Transform, Mesh, Occluder>())
		{
			proc::MeshRenderer::render(entity, progShadowMap);
		}

		progShadowMap.forgo();
		// RN_CHECK(glCullFace(GL_BACK));
	}

	// blur shadowmap: x pass
	{
		RN_FBO_USE(fboShadowMapBlurBuffer);

		RN_SCOPE_DISABLE(GL_DEPTH_TEST);

		progBlurGaussian7.use();

		progBlurGaussian7.var("texSource", fboShadowMapBuffer.bindColorTex(0, 0));
		progBlurGaussian7.var("scale", glm::vec2{1.f, 0.f});

		rn::FBO::mesh.render();

		progBlurGaussian7.forgo();
	}

	// blur shadowmap: y pass
	{
		RN_FBO_USE(fboShadowMapBuffer);

		RN_SCOPE_DISABLE(GL_DEPTH_TEST);

		progBlurGaussian7.use();

		progBlurGaussian7.var("texSource", fboShadowMapBlurBuffer.bindColorTex(0, 0));
		progBlurGaussian7.var("scale", glm::vec2{0.f, 1.f});

		rn::FBO::mesh.render();

		progBlurGaussian7.forgo();
	}

	return shadowMapMVP;
}
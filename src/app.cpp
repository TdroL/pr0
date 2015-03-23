#include "app.hpp"

#include <core/rn.hpp>
#include <core/rn/format.hpp>
#include <core/ngn.hpp>
#include <core/ngn/fs.hpp>
#include <core/ngn/key.hpp>
#include <core/ngn/window.hpp>
#include <core/src/sbm.hpp>
#include <core/src/mem.hpp>
#include <core/event.hpp>
#include <core/util/timer.hpp>
#include <core/util/toggle.hpp>
#include <core/asset/mesh.hpp>
#include <core/phs/frustum.hpp>

#include <app/comp/boundingobject.hpp>
#include <app/comp/direction.hpp>
#include <app/comp/directionallight.hpp>
#include <app/comp/input.hpp>
#include <app/comp/material.hpp>
#include <app/comp/mesh.hpp>
#include <app/comp/name.hpp>
#include <app/comp/occluder.hpp>
#include <app/comp/pointlight.hpp>
#include <app/comp/position.hpp>
#include <app/comp/projection.hpp>
#include <app/comp/rotation.hpp>
#include <app/comp/stencil.hpp>
#include <app/comp/temporaltransform.hpp>
#include <app/comp/transform.hpp>
#include <app/comp/view.hpp>

#include <app/proc/rebuildboundingobjectprocess.hpp>
#include <app/proc/camera.hpp>
#include <app/proc/frustumprocess.hpp>
#include <app/proc/inputprocess.hpp>
#include <app/proc/meshrenderer.hpp>
#include <app/proc/transformprocess.hpp>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <iostream>
#include <cmath>
#include <limits>

namespace fs = ngn::fs;
namespace key = ngn::key;
namespace win = ngn::window;

using namespace comp;
using namespace std;

util::Timer sunTimer{};

util::Toggle toggleSSAO{"SSAO (b)", 1};
util::Toggle toggleSSAOBlur{"SSAOBlur (n)", 2, 3};
util::Toggle toggleDebugPreview{"DebugPreview (m)", 0};
util::Toggle toggleZPreview{"ZPreview (,)", 0};
util::Toggle toggleLights{"Lights (;)", 1};
util::Toggle toggleColor{"Color (/)", 0};

void App::init()
{
	/* Init scene objects */

	clog << "Init shader programs:" << endl;
	initProg();

	clog << "Init frame buffers:" << endl;
	initFB();

	clog << "Init scene objects:" << endl;
	initScene();

	ssao.init(ecs::get<Projection>(cameraId));

	profRender.init();
	profGBuffer.init();
	profDirectionalLight.init();
	profPointLight.init();
	profFlatLight.init();
	profSSAO.init();
}

void App::initProg()
{
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
		progTexPreview.load("rn/fbo.frag", "rn/fbo.vert");
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
		progSSAOBlit.load("lighting/ssaoBlit.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}
}

void App::initFB()
{
	// fbGBuffer
	{
		auto texMaterial = make_shared<rn::Tex2D>("App::fbGBuffer.color[0]");
		texMaterial->width = win::width;
		texMaterial->height = win::height;
		texMaterial->internalFormat = rn::format::RGBA16F.layout;
		texMaterial->reload();

		fbGBuffer.attachColor(0, texMaterial);
	}

	{
		auto texNormals = make_shared<rn::Tex2D>("App::fbGBuffer.color[1]");

		texNormals->width = win::width;
		texNormals->height = win::height;
		texNormals->internalFormat = rn::format::RG16F.layout;
		texNormals->reload();

		fbGBuffer.attachColor(1, texNormals);
	}

	{
		auto texDepth = make_shared<rn::Tex2D>("App::fbGBuffer.depth");

		texDepth->width = win::width;
		texDepth->height = win::height;
		texDepth->internalFormat = rn::format::D24S8.layout;
		texDepth->reload();

		fbGBuffer.attachDepth(texDepth);
	}

	fbGBuffer.reload();

	// fbShadowMap
	{
		auto texDebug = make_shared<rn::Tex2D>("App::fbShadowMap.color[0]");
		texDebug->width = 1024;
		texDebug->height = 1024;
		texDebug->wrapS = rn::WRAP_BORDER;
		texDebug->wrapT = rn::WRAP_BORDER;
		texDebug->borderColor = glm::vec4{1.f};
		texDebug->internalFormat = rn::format::RGB32F.layout;
		texDebug->reload();

		fbShadowMap.attachColor(0, texDebug);

		auto texDepth = make_shared<rn::Tex2D>("App::fbShadowMap.depth");
		texDepth->width = 1024;
		texDepth->height = 1024;
		texDepth->wrapS = rn::WRAP_BORDER;
		texDepth->wrapT = rn::WRAP_BORDER;
		texDepth->borderColor = glm::vec4{1.f};
		texDepth->internalFormat = rn::format::D32F.layout;
		texDepth->reload();

		fbShadowMap.attachDepth(texDepth);
	}

	fbShadowMap.clearColor = glm::vec4{1.f};
	fbShadowMap.reload();

	// fbShadowMapBlur
	{
		auto texDebug = make_shared<rn::Tex2D>("App::fbShadowMapBlur.color[0]");
		texDebug->width = 1024;
		texDebug->height = 1024;
		texDebug->internalFormat = rn::format::RGB32F.layout;
		texDebug->reload();

		fbShadowMapBlur.attachColor(0, texDebug);
	}

	fbShadowMapBlur.reload();
}

void App::initScene()
{
	try
	{
		scene.reload();
	}
	catch (const string &e)
	{
		cerr << e << endl;
	}

	/* Create camera */
	{
		cameraId = ecs::create();

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

		auto resizeCallback = [&] ()
		{
			auto &projection = ecs::get<Projection>(cameraId);

			projection.aspect = static_cast<float>(win::width) / static_cast<float>(win::height);
			projection.matrix = glm::perspective(glm::radians(projection.fovy), projection.aspect, projection.zNear, projection.zFar);
			projection.invMatrix = glm::inverse(projection.matrix);

			progGBuffer.uniform("P", projection.matrix);

			progPointLight.uniform("invP", projection.invMatrix);
			progDirectionalLight.uniform("invP", projection.invMatrix);
			progSSAOBlit.uniform("invP", projection.invMatrix);

			glm::mat4 previewM{1.f};
			previewM = glm::translate(previewM, glm::vec3{0.75f, 0.75f, 0.f});
			previewM = glm::scale(previewM, glm::vec3{0.25f / projection.aspect, 0.25f, 0.f});
			// previewM = glm::scale(previewM, glm::vec3{0.25f});

			progShadowMapPreview.uniform("M", previewM);
			progBlurPreview.uniform("M", previewM);
		};

		resizeCallback();

		event::subscribe<win::WindowResizeEvent>([&resizeCallback] (const win::WindowResizeEvent &)
		{
			resizeCallback();
		});
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
		position.y = 0.5f;
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
		position.y = 0.25f;
		position.z = -0.35f;

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
		light.direction = glm::vec3{0.5f, 0.25f, 1.f};
		light.color = glm::vec4{0.8f, 0.8f, 0.8f, 1.f};
		light.intensity = 4.f;

		auto &transform = ecs::get<Transform>(lightIds[4]);
		transform.translation = light.direction * 100.f;
		transform.scale = glm::vec3{10.f};

		ecs::get<Mesh>(lightIds[4]).id = asset::mesh::load("sphere.sbm");
	}

	for (auto &entity : ecs::findWith<Name, Transform, Mesh>())
	{
		proc::RebuildBoundingObjectProcess::update(entity);
	}
}

void App::update()
{
	for (auto &entity : ecs::findWith<Input>())
	{
		proc::InputProcess::update(entity);

		if (ecs::has<TemporalTransform, Transform>(entity))
		{
			proc::TransformProcess::update(entity);
		}
	}

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

	if (key::hit('b'))
	{
		toggleSSAO.change();
	}

	if (key::hit('n'))
	{
		toggleSSAOBlur.change();
	}

	if (key::hit('m'))
	{
		toggleDebugPreview.change();
	}

	if (key::hit(','))
	{
		toggleZPreview.change();
	}

	if (key::hit(';'))
	{
		toggleLights.change();
	}

	if (key::hit('/'))
	{
		toggleColor.change();
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
		position.x = lightPositionDelta.x * sin(ngn::ct * 0.5f);
		position.y = lightPositionDelta.y * sin(ngn::ct);
		position.z = lightPositionDelta.z * cos(ngn::ct * 0.5f);

		auto &transform = ecs::get<Transform>(lightIds[1]);
		transform.translation = position;
	}

	{
		glm::vec3 lightPositionDelta{1.f, 0.25f, 1.f};

		auto &directionalLight = ecs::get<DirectionalLight>(lightIds[4]);
		directionalLight.direction.x = lightPositionDelta.x * sin(sunTimer.timed);
		directionalLight.direction.z = lightPositionDelta.z * cos(sunTimer.timed);

		auto &transform = ecs::get<Transform>(lightIds[4]);
		const auto &camPosition = ecs::get<Position>(cameraId).position;
		transform.translation = camPosition + directionalLight.direction * 100.f;
	}

	sunTimer.update();

	for (auto &entity : ecs::findWith<Transform, Mesh>())
	{
		proc::RebuildBoundingObjectProcess::update(entity);
	}
}

void App::render()
{
	profRender.start();

	RN_CHECK(glClearColor(0.f, 0.f, 0.f, 0.f));
	RN_CHECK(glClearDepth(numeric_limits<GLfloat>::max()));
	RN_CHECK(glClearStencil(0));
	RN_CHECK(glStencilMask(0xF));
	RN_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

	profGBuffer.start();
	gBufferPass();
	profGBuffer.stop();

	fbGBuffer.blit(nullptr, rn::BUFFER_STENCIL);

	if (!toggleLights.value)
	{
		fbGBuffer.blit(nullptr, rn::BUFFER_COLOR);
	}

	profSSAO.start();
	ssaoPass();
	profSSAO.stop();

	if (toggleLights.value)
	{
		profDirectionalLight.start();
		directionalLightsPass();
		profDirectionalLight.stop();

		// profPointLight.start();
		// pointLightsPass();
		// profPointLight.stop();

		profFlatLight.start();
		flatLightPass();
		profFlatLight.stop();
	}

	if (toggleDebugPreview.value)
	{
		RN_SCOPE_DISABLE(GL_BLEND);
		RN_SCOPE_ENABLE(GL_STENCIL_TEST);
		RN_CHECK(glStencilFunc(GL_EQUAL, Stencil::MASK_SHADED, Stencil::MASK_ALL));
		RN_CHECK(glStencilMask(0x0));
		RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		progSSAOBlit.use();

		progSSAOBlit.var("texSource", ssao.fbAO.color(0)->bind(0));
		progSSAOBlit.var("texColor", fbGBuffer.color(0)->bind(1));
		progSSAOBlit.var("texNormal", fbGBuffer.color(1)->bind(2));
		progSSAOBlit.var("texDepth", fbGBuffer.depth()->bind(3));
		progSSAOBlit.var("texZ", ssao.fbZ.color(0)->bind(4));

		rn::Mesh::quad.render();

		progSSAOBlit.forgo();
	}

	if (toggleZPreview.value)
	{
		RN_SCOPE_DISABLE(GL_BLEND);
		RN_SCOPE_DISABLE(GL_DEPTH_TEST);
		RN_SCOPE_DISABLE(GL_STENCIL_TEST);

		progTexPreview.use();
		progTexPreview.var("texSource", ssao.fbZ.color(0)->bind(0));

		rn::Mesh::quad.render();

		progTexPreview.forgo();
	}

	{
		progShadowMapPreview.use();
		progShadowMapPreview.var("texSource", fbShadowMap.color(0)->bind(0));

		rn::Mesh::quad.render();

		progShadowMapPreview.forgo();
	}

	profRender.stop();
}

void App::gBufferPass()
{
	RN_SCOPE_DISABLE(GL_BLEND);
	RN_SCOPE_ENABLE(GL_STENCIL_TEST);

	RN_CHECK(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
	RN_CHECK(glStencilMask(0xF));

	RN_FB_BIND(fbGBuffer);
	fbGBuffer.clear(rn::BUFFER_COLOR | rn::BUFFER_DEPTH | rn::BUFFER_STENCIL);

	const auto &V = ecs::get<View>(cameraId).matrix;
	const auto &P = ecs::get<Projection>(cameraId).matrix;

	const phs::Frustum frustum{P * V};

	progGBuffer.use();
	progGBuffer.var("V", V);

	for (auto &entity : ecs::findWith<Transform, Mesh, Material>())
	{
		if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
		{
			continue;
		}

		GLint ref = ecs::has<Stencil>(entity) ? ecs::get<Stencil>(entity).ref : 0;

		RN_CHECK(glStencilFunc(GL_ALWAYS, ref, 0xF));

		proc::MeshRenderer::render(entity, progGBuffer);
	}

	// draw light meshes
	RN_CHECK(glStencilFunc(GL_ALWAYS, Stencil::MASK_FLAT, 0xF));

	progGBuffer.var("matShininess", 0.f);

	for (auto &entity : ecs::findWith<Transform, Mesh, PointLight>())
	{
		if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
		{
			continue;
		}

		progGBuffer.var("matDiffuse", ecs::get<PointLight>(entity).color);
		proc::MeshRenderer::render(entity, progGBuffer);
	}

	for (auto &entity : ecs::findWith<Transform, Mesh, DirectionalLight>())
	{
		if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
		{
			continue;
		}

		progGBuffer.var("matDiffuse", ecs::get<DirectionalLight>(entity).color);
		proc::MeshRenderer::render(entity, progGBuffer);
	}

	progGBuffer.forgo();
}

void App::directionalLightsPass()
{
	const auto &view = ecs::get<View>(cameraId);
	const glm::mat4 &V = view.matrix;
	const glm::mat4 &invV = view.invMatrix;

	progDirectionalLight.var("invV", invV);

	for (auto &entity : ecs::findWith<DirectionalLight>())
	{
		glm::mat4 shadowMapMVP = genShadowMap(entity);

		const auto &light = ecs::get<DirectionalLight>(entity);

		RN_CHECK(glBlendFunc(GL_ONE, GL_ONE));
		RN_SCOPE_ENABLE(GL_STENCIL_TEST);
		RN_CHECK(glStencilFunc(GL_EQUAL, Stencil::MASK_SHADED, Stencil::MASK_ALL));
		RN_CHECK(glStencilMask(0x0));

		// global lighting
		{

			progDirectionalLight.use();

			progDirectionalLight.var("useColor", toggleColor.value);

			progDirectionalLight.var("lightColor", light.color);
			progDirectionalLight.var("lightDirection", glm::mat3{V} * light.direction);
			progDirectionalLight.var("lightIntensity", light.intensity);
			progDirectionalLight.var("shadowmapMVP", shadowMapMVP);

			GLsizei unit = 0;

			progDirectionalLight.var("texColor", fbGBuffer.color(0)->bind(unit++));
			progDirectionalLight.var("texNormal", fbGBuffer.color(1)->bind(unit++));
			progDirectionalLight.var("texDepth", fbGBuffer.depth()->bind(unit++));
			progDirectionalLight.var("texAO", ssao.fbAO.color(0)->bind(unit++));

			progDirectionalLight.var("shadowMoments", fbShadowMap.color(0)->bind(unit++));
			progDirectionalLight.var("shadowDepth", fbShadowMap.depth()->bind(unit++));

			rn::Mesh::quad.render();

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
			RN_CHECK(glStencilFunc(GL_EQUAL, Stencil::MASK_SHADED, Stencil::MASK_ALL));
			RN_CHECK(glStencilMask(0x0));

			RN_CHECK(glBlendFunc(GL_ONE, GL_ONE));

			progPointLight.use();
			progPointLight.var("lightPosition", lightPosition);
			progPointLight.var("lightColor", light.color);
			progPointLight.var("lightIntensity", light.intensity);
			progPointLight.var("lightLinearAttenuation", light.linearAttenuation);
			progPointLight.var("lightQuadraticAttenuation", light.quadraticAttenuation);

			progPointLight.var("texColor", fbGBuffer.color(0)->bind(0));
			progPointLight.var("texNormal", fbGBuffer.color(1)->bind(1));
			progPointLight.var("texDepth", fbGBuffer.depth()->bind(2));

			// shadowMapBuffer.colors[0].bind(3);
			// shadowMapBuffer.depth.tex.bind(4);

			// progPointLight.var("shadowMoments", 3);
			// progPointLight.var("shadowDepth", 4);

			rn::Mesh::quad.render();

			progPointLight.forgo();

			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		}
	}
}

void App::flatLightPass()
{
	RN_SCOPE_ENABLE(GL_STENCIL_TEST);
	RN_CHECK(glStencilFunc(GL_EQUAL, Stencil::MASK_FLAT, Stencil::MASK_ALL));
	RN_CHECK(glStencilMask(0x0));

	progFlatLight.use();

	progFlatLight.var("texColor", fbGBuffer.color(0)->bind(0));

	rn::Mesh::quad.render();

	progFlatLight.forgo();
}

void App::ssaoPass()
{
	ssao.clear();

	if (!toggleSSAO.value)
	{
		return;
	}

	ssao.genMipMaps(fbGBuffer);
	ssao.computeAO(fbGBuffer);
	ssao.blur(fbGBuffer);
}

glm::mat4 App::genShadowMap(ecs::Entity lightId)
{
	const auto &light = ecs::get<DirectionalLight>(lightId);

	const auto shadowMapP = glm::ortho(-10.f, 10.f, -10.f, 10.f, -10.f, 20.f);
	const auto shadowMapV = glm::lookAt(glm::normalize(light.direction), glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.f, 1.f, 0.f});
	const auto shadowMapVP = shadowMapP * shadowMapV;

	const phs::Frustum frustum{shadowMapVP};

	{
		RN_FB_BIND(fbShadowMap);

		RN_SCOPE_DISABLE(GL_BLEND);

		// RN_CHECK(glCullFace(GL_FRONT));

		fbShadowMap.clear(rn::BUFFER_COLOR | rn::BUFFER_DEPTH);

		progShadowMap.use();
		progShadowMap.var("P", shadowMapP);
		progShadowMap.var("V", shadowMapV);

		for (auto &entity : ecs::findWith<Transform, Mesh, Occluder>())
		{
			if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
			{
				continue;
			}

			proc::MeshRenderer::render(entity, progShadowMap);
		}

		progShadowMap.forgo();
		// RN_CHECK(glCullFace(GL_BACK));
	}

	// blur shadowmap: x pass
	{
		RN_FB_BIND(fbShadowMapBlur);

		RN_SCOPE_DISABLE(GL_DEPTH_TEST);

		progBlurGaussian7.use();

		progBlurGaussian7.var("texSource", fbShadowMap.color(0)->bind(0));
		progBlurGaussian7.var("scale", glm::vec2{1.f, 0.f});

		rn::Mesh::quad.render();

		progBlurGaussian7.forgo();
	}

	// blur shadowmap: y pass
	{
		RN_FB_BIND(fbShadowMap);

		RN_SCOPE_DISABLE(GL_DEPTH_TEST);

		progBlurGaussian7.use();

		progBlurGaussian7.var("texSource", fbShadowMapBlur.color(0)->bind(0));
		progBlurGaussian7.var("scale", glm::vec2{0.f, 1.f});

		rn::Mesh::quad.render();

		progBlurGaussian7.forgo();
	}

	return shadowMapVP;
}
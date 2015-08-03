#include "app.hpp"

#include <core/asset/mesh.hpp>
#include <core/event.hpp>
#include <core/ngn.hpp>
#include <core/ngn/fs.hpp>
#include <core/ngn/key.hpp>
#include <core/ngn/window.hpp>
#include <core/phs/frustum.hpp>
#include <core/rn.hpp>
#include <core/rn/format.hpp>
#include <core/src/mem.hpp>
#include <core/src/sbm.hpp>
#include <core/util/count.hpp>
#include <core/util/timer.hpp>
#include <core/util/toggle.hpp>

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

#include <app/proc/camera.hpp>
#include <app/proc/frustumprocess.hpp>
#include <app/proc/inputprocess.hpp>
#include <app/proc/meshrenderer.hpp>
#include <app/proc/rebuildboundingobjectprocess.hpp>
#include <app/proc/transformprocess.hpp>

#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <sstream>
#include <cmath>
#include <limits>
#include <iomanip>

namespace fs = ngn::fs;
namespace key = ngn::key;
namespace win = ngn::window;

using namespace comp;
using namespace std;

util::Timer sunTimer{};

util::Toggle toggleSSAO{"SSAO (b)", 1};
util::Toggle toggleSSAOBlur{"SSAOBlur (n)", 1};
util::Toggle toggleDebugPreview{"DebugPreview (m)", 0};
util::Toggle toggleZPreview{"ZPreview (,)", 0};
util::Toggle toggleLights{"Lights (;)", 1};
util::Toggle toggleColor{"Color (/)", 0};
util::Toggle toggleCascade{"Cascade (')", 0, 4};
util::Toggle toggleUseSmartSplitting{"UseSmartSplitting (])", 1};
util::Toggle toggleCalculateMatrices{"CalculateMatrices ([)", 1};

const win::Mode modes[]
{
	win::Mode::windowed,
	win::Mode::borderless,
	win::Mode::fullscreen
};

const string modeNames[]
{
	"windowed",
	"borderless",
	"fullscreen",
};

size_t currentMode = 0;

const int vsyncs[]
{
	-1, // progressive
	 0, // off
	 1, // on
};

const string vsyncNames[]
{
	"progressive",
	"off",
	"on",
};

size_t currentVsync = 0;

void App::init()
{
	font1.load("DejaVu/DejaVuSansMono.ttf");
	font2.load("DejaVu/DejaVuSansMono.ttf");

	clog << "Init shader programs:" << endl;
	initProg();

	clog << "Init frame buffers:" << endl;
	initFB();

	clog << "Init scene objects:" << endl;
	initScene();

	ssao.init(ecs::get<Projection>(cameraId));
	csm.init();

	profRender.init();
	profGBuffer.init();
	profDirectionalLight.init();
	profPointLight.init();
	profFlatLight.init();
	profSSAO.init();

	/* Test: switch to window mode */
	UTIL_DEBUG
	{
		currentMode = 0;
		currentVsync = 1;

		clog << "Test: switching to " << modeNames[currentMode] << " " << vsyncNames[currentVsync] << endl;

		win::switchMode(modes[currentMode], vsyncs[currentVsync]);
		rn::reloadSoftAll();
	}
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
		texMaterial->width = win::internalWidth;
		texMaterial->height = win::internalHeight;
		texMaterial->internalFormat = rn::format::RGBA8.layout;
		texMaterial->reload();

		fbGBuffer.attachColor(0, texMaterial);
	}

	{
		auto texNormals = make_shared<rn::Tex2D>("App::fbGBuffer.color[1]");

		texNormals->width = win::internalWidth;
		texNormals->height = win::internalHeight;
		texNormals->internalFormat = rn::format::RG16F.layout;
		texNormals->reload();

		fbGBuffer.attachColor(1, texNormals);
	}

	{
		auto texZ = make_shared<rn::Tex2D>("App::fbGBuffer.color[2]");
		texZ->width = win::internalWidth;
		texZ->height = win::internalHeight;
		// texZ->internalFormat = rn::format::R32F.layout;
		texZ->internalFormat = rn::format::RGB32F.layout;
		texZ->reload();

		fbGBuffer.attachColor(2, texZ);
	}

	{
		auto texDepth = make_shared<rn::Tex2D>("App::fbGBuffer.depth");
		texDepth->width = win::internalWidth;
		texDepth->height = win::internalHeight;
		texDepth->internalFormat = rn::format::D32FS8.layout;
		texDepth->reload();

		fbGBuffer.attachDepth(texDepth);
	}

	fbGBuffer.reload();

	// fbScreen
	{
		auto texColor = make_shared<rn::Tex2D>("App::fbScreen.color[0]");
		texColor->width = win::internalWidth;
		texColor->height = win::internalHeight;
		texColor->internalFormat = rn::format::RGBA16F.layout;
		texColor->reload();

		fbScreen.attachColor(0, texColor);
	}

	{
		auto texDepth = make_shared<rn::Tex2D>("App::fbScreen.depth");
		texDepth->width = win::internalWidth;
		texDepth->height = win::internalHeight;
		texDepth->internalFormat = rn::format::D32FS8.layout;
		texDepth->reload();

		fbScreen.attachDepth(texDepth);
	}

	fbScreen.clearColor = glm::vec4{0.f, 0.f, 0.f, 1.f};
	fbScreen.reload();

	// fbUI
	{
		auto texColor = make_shared<rn::Tex2D>("App::fbUI.color[0]");
		texColor->width = win::internalWidth;
		texColor->height = win::internalHeight;
		texColor->internalFormat = rn::format::RGBA8.layout;
		texColor->reload();

		fbUI.attachColor(0, texColor);
	}

	fbUI.clearColor = glm::vec4{0.f, 0.f, 0.f, 0.f};
	fbUI.reload();

	event::subscribe<win::WindowResizeEvent>([&] (const win::WindowResizeEvent &)
	{
		/*
		auto texMaterial = dynamic_cast<rn::Tex2D *>(fbGBuffer.color(0));
		texMaterial->width = win::internalWidth;
		texMaterial->height = win::internalHeight;

		auto texNormals = dynamic_cast<rn::Tex2D *>(fbGBuffer.color(1));
		texNormals->width = win::internalWidth;
		texNormals->height = win::internalHeight;

		auto texZ = dynamic_cast<rn::Tex2D *>(fbGBuffer.color(2));
		texZ->width = win::internalWidth;
		texZ->height = win::internalHeight;

		auto texDepth = dynamic_cast<rn::Tex2D *>(fbGBuffer.depth());
		texDepth->width = win::internalWidth;
		texDepth->height = win::internalHeight;

		fbGBuffer.width = win::internalWidth;
		fbGBuffer.height = win::internalHeight;
		fbGBuffer.reload();

		auto texDepth = dynamic_cast<rn::Tex2D *>(fbScreen.color(0));
		texColor->width = win::internalWidth;
		texColor->height = win::internalHeight;

		auto texDepth = dynamic_cast<rn::Tex2D *>(fbScreen.depth());
		texDepth->width = win::internalWidth;
		texDepth->height = win::internalHeight;

		fbScreen.width = win::internalWidth;
		fbScreen.height = win::internalHeight;
		fbScreen.reload();
		*/
	});

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

		auto &projection = ecs::get<Projection>(cameraId);
		projection.zFar = 500.f;

		auto resizeCallback = [&] ()
		{
			auto &projection = ecs::get<Projection>(cameraId);

			projection.aspect = static_cast<float>(win::internalWidth) / static_cast<float>(win::internalHeight);
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
		light.ambient = glm::vec4{0.f, 0.36f, 0.5f, 1.f};
		light.color = glm::vec4{0.8f, 0.8f, 0.8f, 1.f};
		light.direction = glm::vec3{0.5f, 0.25f, 1.f};
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

	{
		glm::vec3 translate{0.0};
		glm::vec3 rotate{0.0};

		proc::Camera::update(cameraId, translate, rotate);
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

		float translateSpeed = (key::pressed(KEY_LSHIFT) ? 0.1 : 20.0);
		float rotateSpeed = (key::pressed(KEY_LSHIFT) ? 5.0 : 90.0);

		translate *= translateSpeed * ngn::dt;
		rotate *= rotateSpeed * ngn::dt;

		proc::Camera::update(cameraId, translate, rotate);
	}

	if (key::hit(KEY_ESCAPE))
	{
		win::close();
	}

	if (key::hit(KEY_F4))
	{
		cout << "Reloading scene..." << endl;

		try
		{
			scene.reload();
			cout << "done" << endl;
		}
		catch (const string &e)
		{
			cerr << e << endl;
		}
	}

	if (key::hit(KEY_F5))
	{
		cout << "Reloading shaders..." << endl;
		try
		{
			rn::Program::reloadAll();
			cout << "done" << endl;
		}
		catch (string e)
		{
			cerr << "  - " << e << endl;
		}
	}

	if (key::hit(KEY_F6))
	{
		cout << "Reloading meshes..." << endl;
		rn::Mesh::reloadAll();
		cout << "done" << endl;
	}

	if (key::hit(KEY_F7))
	{
		cout << "Reloading fonts..." << endl;
		rn::Font::reloadAll();
		cout << "done" << endl;
	}

	if (key::hit(KEY_F8))
	{
		cout << "Reloading FBs..." << endl;
		rn::FB::reloadAll();
		cout << "done" << endl;
	}

	if (key::hit(KEY_F9))
	{
		cout << "Reloading textures..." << endl;
		rn::Tex2D::reloadAll();
		cout << "done" << endl;
	}

	if (key::hit(KEY_F10))
	{
		currentVsync = (currentVsync + 1) % util::countOf(vsyncs);

		cout << "Switching vsync mode to \"" << vsyncNames[currentVsync] << "\" (" << vsyncs[currentVsync] << ") ..." << endl;
		win::switchMode(modes[currentMode], vsyncs[currentVsync]);
		rn::reloadSoftAll();
		cout << "done" << endl;
	}

	if (key::hit(KEY_F11))
	{
		cout << "Switching window mode..." << endl;
		currentMode = (currentMode + 1) % util::countOf(modes);
		win::switchMode(modes[currentMode], vsyncs[currentVsync]);
		cout << "done" << endl;

		cout << "Soft reloading GL..." << endl;
		rn::reloadSoftAll();
		cout << "done" << endl;
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

	if (key::hit('\''))
	{
		toggleCascade.change();
	}

	if (key::hit(']'))
	{
		toggleUseSmartSplitting.change();
	}

	if (key::hit('['))
	{
		toggleCalculateMatrices.change();
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
		directionalLight.direction.x = lightPositionDelta.x * sin(sunTimer.timed); //  + 3.151592f / 4.f
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

	fbScreen.bind();

	RN_CHECK(glClearColor(0.f, 0.f, 0.f, 0.f));
	RN_CHECK(glClearDepth(numeric_limits<GLfloat>::max()));
	RN_CHECK(glClearStencil(0));
	RN_CHECK(glStencilMask(0xF));
	RN_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

	profGBuffer.start();
	gBufferPass();
	profGBuffer.stop();

	// fbScreen.clear(rn::BUFFER_COLOR);
	fbGBuffer.blit(fbScreen, rn::BUFFER_STENCIL);
	// fbGBuffer.blit(nullptr, rn::BUFFER_STENCIL);

	if (!toggleLights.value)
	{
		fbGBuffer.blit(fbScreen, rn::BUFFER_COLOR);
		// fbGBuffer.blit(nullptr, rn::BUFFER_COLOR);
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

		// fbScreen.blit(nullptr, rn::BUFFER_COLOR/*, rn::MAG_LINEAR*/);
	}

	if (toggleDebugPreview.value)
	{
		RN_SCOPE_DISABLE(GL_BLEND);
		RN_SCOPE_ENABLE(GL_STENCIL_TEST);
		RN_CHECK(glStencilFunc(GL_EQUAL, Stencil::MASK_SHADED, Stencil::MASK_ALL));
		RN_CHECK(glStencilMask(0x0));

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
		// progShadowMapPreview.var("texSource", fbShadowMap.color(0)->bind(0));
		// progShadowMapPreview.var("texSource", csm.fbShadows[0].color(0)->bind(0));

		auto unit = csm.texDepths->bind(0);
		RN_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_NONE));

		progShadowMapPreview.var("texSource", unit);
		progShadowMapPreview.var("layer", toggleCascade.value);

		rn::Mesh::quad.render();

		progShadowMapPreview.forgo();

		RN_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
		RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, csm.texDepths->compareFunc), rn::getEnumName(csm.texDepths->compareFunc));
	}

	fbScreen.unbind();

	fbScreen.blit(nullptr, rn::BUFFER_COLOR);

	profRender.stop();

	double ft = ngn::time() - ngn::ct;

	fbUI.bind();
	fbUI.clear(rn::BUFFER_COLOR);

	ostringstream oss;
	oss << setprecision(4) << fixed;
	oss << "dt=" << ngn::dt * 1000.0 << "ms\n";
	oss << "ft=" << ft * 1000.0 << "ms\n";
	oss << "fps=" << 1.0 / ngn::dt << "\n";
	oss << "fps=" << 1.0 / ft << " (frame)\n";
	oss << "triangles=" << rn::stats.triangles << "\n";
	oss << "\n";

	oss << "render=" << profRender.ms() << "ms (" << 1000.0 / profRender.ms() << ")\n";
	oss << "  GBuffer=" << profGBuffer.ms() << "ms\n";
	oss << "  DirectionalLight=" << profDirectionalLight.ms() << "ms\n";
	oss << "  PointLight=" << profPointLight.ms() << "ms\n";
	oss << "  FlatLight=" << profFlatLight.ms() << "ms\n";
	oss << "  SSAO=" << profSSAO.ms() << "ms\n";
	oss << "    Z=" << ssao.profZ.ms() << "ms\n";
	oss << "    MipMaps=" << ssao.profMipMaps.ms() << "ms\n";
	oss << "    AO=" << ssao.profAO.ms() << "ms\n";
	oss << "    Blur=" << ssao.profBlur.ms() << "ms\n";
	oss << "  CSM=???ms\n";
	oss << "    Render=" << csm.profRender.ms() << "ms\n";
	oss << "    Blur=" << csm.profBlur.ms() << "ms\n";

	oss << "\n";
	oss << "F4 - reload scene\n";
	oss << "F5 - reload shaders\n";
	oss << "F6 - reload meshes\n";
	oss << "F7 - reload fonts\n";
	oss << "F8 - reload FBOs\n";
	oss << "F9 - reload textures\n";
	oss << "F10 - change vsync mode (current: " << vsyncNames[currentVsync] << ")\n";
	oss << "F11 - change window mode (current: " << modeNames[currentMode] << ")\n";
	oss << "\n";
	oss << "Movement: W, A, S, D\n";
	oss << "Camera: arrows\n";
	oss << "Point Light: Keypad 8, 4, 5, 6\n\n";
	oss << "Toggles:\n";

	for (auto &toggle : util::Toggle::collection)
	{
		oss << "  " << toggle->toggleName << " = " << toggle->value << "\n";
	}

	font1.render(oss.str());

	oss.str(""s);
	oss.clear();

	oss << "CSM:\n";
	oss << "  V[0][0]=" << glm::to_string(csm.Vs[0][0]) << "\n";
	oss << "  V[0][1]=" << glm::to_string(csm.Vs[0][1]) << "\n";
	oss << "  V[0][2]=" << glm::to_string(csm.Vs[0][2]) << "\n";
	oss << "  V[0][3]=" << glm::to_string(csm.Vs[0][3]) << "\n";
	oss << "\n";
	oss << "  P[0][0]=" << glm::to_string(csm.Ps[0][0]) << "\n";
	oss << "  P[0][1]=" << glm::to_string(csm.Ps[0][1]) << "\n";
	oss << "  P[0][2]=" << glm::to_string(csm.Ps[0][2]) << "\n";
	oss << "  P[0][3]=" << glm::to_string(csm.Ps[0][3]) << "\n";

	font2.position.x = -0.125f;
	font2.render(oss.str());

	fbUI.unbind();

	RN_SCOPE_ENABLE(GL_BLEND);
	RN_SCOPE_DISABLE(GL_DEPTH_TEST);
	RN_CHECK(glBlendFunc(GL_ONE, GL_ONE));

	progFBOBlit.use();
	progFBOBlit.uniform("texSource", fbUI.color(0)->bind(0));
	rn::Mesh::quad.render();
	progFBOBlit.forgo();

	RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
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
	const auto &projection = ecs::get<Projection>(cameraId);
	const auto &P = projection.matrix;
	const auto &view = ecs::get<View>(cameraId);
	const auto &V = view.matrix;
	const auto &invV = view.invMatrix;

	const phs::Frustum frustum{P * V};

	progDirectionalLight.var("invV", invV);
	progDirectionalLight.var("zNear", projection.zNear);
	progDirectionalLight.var("zFar", projection.zFar);

	for (auto &entity : ecs::findWith<DirectionalLight>())
	{
		// glm::mat4 shadowMapMVP = makeShadowMap(entity, frustum);
		csm.useSmartSplitting = toggleUseSmartSplitting.value;
		if (toggleCalculateMatrices.value) {
			csm.calculateMatrices(cameraId, entity);
		}

		csm.renderCascades();

		const auto &light = ecs::get<DirectionalLight>(entity);

		{
			// RN_FB_BIND(fbScreen);
			// RN_SCOPE_DISABLE(GL_DEPTH_TEST);

			RN_CHECK(glBlendFunc(GL_ONE, GL_ONE));
			RN_SCOPE_ENABLE(GL_STENCIL_TEST);
			RN_CHECK(glStencilFunc(GL_EQUAL, Stencil::MASK_SHADED, Stencil::MASK_ALL));
			RN_CHECK(glStencilMask(0x0));

			progDirectionalLight.use();

			progDirectionalLight.var("useColor", toggleColor.value);

			progDirectionalLight.var("lightAmbient", light.ambient);
			progDirectionalLight.var("lightColor", light.color);
			progDirectionalLight.var("lightDirection", glm::mat3{V} * light.direction);
			progDirectionalLight.var("lightIntensity", light.intensity);
			// progDirectionalLight.var("shadowmapMVP", shadowMapMVP);
			// progDirectionalLight.var("csmMVP", csm.Ps[0] * csm.V);

			std::vector<float> csmRadiuses2{};
			csmRadiuses2.resize(csm.radiuses.size());

			for (size_t i = 0; i < csmRadiuses2.size(); i++)
			{
				csmRadiuses2[i] = csm.radiuses[i] * csm.radiuses[i];
			}

			std::vector<glm::vec3> csmCenters{};
			csmCenters.resize(csm.centers.size());

			for (size_t i = 0; i < csmCenters.size(); i++)
			{
				csmCenters[i] = glm::vec3{V  * glm::vec4{csm.centers[i], 1.f}};
			}

			std::vector<glm::mat4> csmMVP{};
			csmMVP.resize(csm.Ps.size());

			for (size_t i = 0; i < csmMVP.size(); i++)
			{
				csmMVP[i] = csm.Ps[i] * csm.Vs[i];
			}

			GLsizei unit = 0;
			progDirectionalLight.var("texColor", fbGBuffer.color(0)->bind(unit++));
			progDirectionalLight.var("texNormal", fbGBuffer.color(1)->bind(unit++));
			progDirectionalLight.var("texZ", fbGBuffer.color(2)->bind(unit++));
			progDirectionalLight.var("texDepth", fbGBuffer.depth()->bind(unit++));
			progDirectionalLight.var("texAO", ssao.fbAO.color(0)->bind(unit++));

			progDirectionalLight.var("texShadowMoments", fbShadowMap.color(0)->bind(unit++));
			progDirectionalLight.var("texShadowDepth", fbShadowMap.depth()->bind(unit++));
			// progDirectionalLight.var("texCSM", csm.fbShadows[0].color(0)->bind(unit++));

			progDirectionalLight.var("csmSplits", static_cast<GLint>(csm.splits));
			progDirectionalLight.var("csmCascades", csm.cascades.data(), csm.cascades.size());
			progDirectionalLight.var("csmRadiuses2", csmRadiuses2.data(), csmRadiuses2.size());
			progDirectionalLight.var("csmCenters", csmCenters.data(), csmCenters.size());
			progDirectionalLight.var("csmMVP", csmMVP.data(), csmMVP.size());
			// progDirectionalLight.var("csmTexCascades", csm.texCascades->bind(unit++));
			progDirectionalLight.var("csmTexDepths", csm.texDepths->bind(unit++));

			rn::Mesh::quad.render();

			progDirectionalLight.forgo();

			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		}
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
		// 	RN_FB_BIND(shadowmap);

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
			// RN_FB_BIND(fbScreen);

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
			progPointLight.var("texZ", fbGBuffer.color(2)->bind(2));
			progPointLight.var("texDepth", fbGBuffer.depth()->bind(3));

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

	if (toggleSSAOBlur.value) {
		ssao.blur(fbGBuffer);
	}
}

glm::mat4 App::makeShadowMap(const ecs::Entity &lightId, const phs::Frustum &frustum)
{
	// csm.cameraId = cameraId;

	// csm.calculateMatrices(lightId);
	// csm.renderCascades();

	const auto &cameraPosition = ecs::get<Position>(cameraId).position;
	// const auto &projection = ecs::get<Projection>(cameraId);
	// const auto &view = ecs::get<View>(cameraId);
	// const auto &P = projection.matrix;
	// const auto &V = view.matrix;

	const auto &light = ecs::get<DirectionalLight>(lightId);

	// const auto shadowMapV = glm::lookAt(glm::normalize(light.direction), glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.f, 1.f, 0.f});
	// const auto shadowMapV = glm::lookAt(cameraPosition, cameraPosition - light.direction, glm::vec3{0.f, 1.f, 0.f});

	// const auto invVP = glm::inverse(P * V);
	// glm::vec4 frustumCorners[8] = {
	// 	invVP * glm::vec4{ 1.f,  1.f, -1.f, 1.f}, // rtn
	// 	invVP * glm::vec4{-1.f,  1.f, -1.f, 1.f}, // ltn
	// 	invVP * glm::vec4{ 1.f, -1.f, -1.f, 1.f}, // rbn
	// 	invVP * glm::vec4{-1.f, -1.f, -1.f, 1.f}, // lbn
	// 	invVP * glm::vec4{ 1.f,  1.f,  1.f, 1.f}, // rtf
	// 	invVP * glm::vec4{-1.f,  1.f,  1.f, 1.f}, // ltf
	// 	invVP * glm::vec4{ 1.f, -1.f,  1.f, 1.f}, // rbf
	// 	invVP * glm::vec4{-1.f, -1.f,  1.f, 1.f}, // lbf
	// };

	// frustumCorners[0] /= frustumCorners[0].w;
	// frustumCorners[1] /= frustumCorners[1].w;
	// frustumCorners[2] /= frustumCorners[2].w;
	// frustumCorners[3] /= frustumCorners[3].w;
	// frustumCorners[4] /= frustumCorners[4].w;
	// frustumCorners[5] /= frustumCorners[5].w;
	// frustumCorners[6] /= frustumCorners[6].w;
	// frustumCorners[7] /= frustumCorners[7].w;

	// float t = (10.f - projection.zNear) / (projection.zFar - projection.zNear);

	// frustumCorners[4] = glm::lerp(frustumCorners[0], frustumCorners[4], t);
	// frustumCorners[5] = glm::lerp(frustumCorners[1], frustumCorners[5], t);
	// frustumCorners[6] = glm::lerp(frustumCorners[2], frustumCorners[6], t);
	// frustumCorners[7] = glm::lerp(frustumCorners[3], frustumCorners[7], t);

	// glm::vec4 shadowFrustumCorners[] = {
	// 	shadowMapV * frustumCorners[0],
	// 	shadowMapV * frustumCorners[1],
	// 	shadowMapV * frustumCorners[2],
	// 	shadowMapV * frustumCorners[3],
	// 	shadowMapV * frustumCorners[4],
	// 	shadowMapV * frustumCorners[5],
	// 	shadowMapV * frustumCorners[6],
	// 	shadowMapV * frustumCorners[7]
	// };

	// float shadowFrustumLeft = shadowFrustumCorners[0].x;
	// float shadowFrustumRight = shadowFrustumCorners[0].x;
	// float shadowFrustumBottom = shadowFrustumCorners[0].y;
	// float shadowFrustumTop = shadowFrustumCorners[0].y;
	// float shadowFrustumZNear = shadowFrustumCorners[0].z;
	// float shadowFrustumZFar = shadowFrustumCorners[0].z;

	// cout << "shadowFrustumCorners[rtn]=" << glm::to_string(shadowFrustumCorners[0].xyz()) << endl;
	// cout << "shadowFrustumCorners[ltn]=" << glm::to_string(shadowFrustumCorners[1].xyz()) << endl;
	// cout << "shadowFrustumCorners[rbn]=" << glm::to_string(shadowFrustumCorners[2].xyz()) << endl;
	// cout << "shadowFrustumCorners[lbn]=" << glm::to_string(shadowFrustumCorners[3].xyz()) << endl;
	// cout << "shadowFrustumCorners[rtf]=" << glm::to_string(shadowFrustumCorners[4].xyz()) << endl;
	// cout << "shadowFrustumCorners[ltf]=" << glm::to_string(shadowFrustumCorners[5].xyz()) << endl;
	// cout << "shadowFrustumCorners[rbf]=" << glm::to_string(shadowFrustumCorners[6].xyz()) << endl;
	// cout << "shadowFrustumCorners[lbf]=" << glm::to_string(shadowFrustumCorners[7].xyz()) << endl;

	// for (const auto &corner : shadowFrustumCorners)
	// {
	// 	shadowFrustumLeft = min(shadowFrustumLeft, corner.x);
	// 	shadowFrustumRight = max(shadowFrustumRight, corner.x);
	// 	shadowFrustumBottom = min(shadowFrustumBottom, corner.y);
	// 	shadowFrustumTop = max(shadowFrustumTop, corner.y);
	// 	shadowFrustumZNear = max(shadowFrustumZNear, corner.z);
	// 	shadowFrustumZFar = min(shadowFrustumZFar, corner.z);
	// }

	// glm::vec3 lightDirection = glm::normalize(ecs::get<DirectionalLight>(lightId).direction);
	// float zMax = -numeric_limits<float>::max();

	// for (auto &entity : ecs::findWith<Transform, Mesh, BoundingObject, Occluder>())
	// {
	// 	auto &boundingObject = ecs::get<BoundingObject>(entity);

	// 	float dist = glm::dot(lightDirection, boundingObject.sphere.position) + boundingObject.sphere.radius;

	// 	zMax = max(zMax, dist);
	// }

	// shadowFrustumZNear = max(shadowFrustumZNear, zMax);

	// const auto shadowMapP = glm::ortho(-10.f, 10.f, -10.f, 10.f, -10.f, 20.f);
	// const auto shadowMapP = glm::ortho(shadowFrustumLeft, shadowFrustumRight, shadowFrustumBottom, shadowFrustumTop, -shadowFrustumZNear, -shadowFrustumZFar);

	const auto shadowMapV = glm::lookAt(cameraPosition, cameraPosition - light.direction, glm::vec3{0.f, 1.f, 0.f});

	const auto shadowMapP = glm::ortho(-10.f, 10.f, -10.f, 10.f, -10.f, 20.f);
	const auto shadowMapVP = shadowMapP * shadowMapV;

	const phs::Frustum shadowFrustum{shadowMapVP};

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
			if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, shadowFrustum))
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
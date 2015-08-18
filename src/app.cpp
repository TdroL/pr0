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

util::Toggle toggleDeferred{"Deferred (-)", 0};
util::Toggle toggleSSAO{"SSAO (b)", 0};
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

	profZPrefill.init();
	profSetupLights.init();
	profLighting.init();

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
		progFBBlit.load("rn/fboBlit.frag", "rn/fbo.vert");
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
		progZPrefill.load("lighting/forward/zPrefill.frag", "lighting/forward/zPrefill.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progZDebug.load("lighting/forward/zDebug.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progLightingForward.load("lighting/forward/lighting.frag", "lighting/forward/lighting.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	// try
	// {
	// 	progBlurGaussian7.load("rn/blurGaussian7.frag", "rn/fbo.vert");
	// }
	// catch (const string &e)
	// {
	// 	cerr << "Warning: " << e << endl;
	// }

	// try
	// {
	// 	progShadowMap.load("lighting/shadows/depthVSM.frag", "P.vert");
	// }
	// catch (const string &e)
	// {
	// 	cerr << "Warning: " << e << endl;
	// }

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

	fbScreen.clearColor = glm::vec4{0.f, 0.f, 0.f, 0.f};
	fbScreen.reload();

	// fbZPrefill
	{
		auto texNormals = make_shared<rn::Tex2D>("App::fbZPrefill.color[0]");

		texNormals->width = win::internalWidth;
		texNormals->height = win::internalHeight;
		texNormals->internalFormat = rn::format::RG16F.layout;
		texNormals->reload();

		fbZPrefill.attachColor(0, texNormals);
	}

	{
		auto texNormals = make_shared<rn::Tex2D>("App::fbZPrefill.color[1]");

		texNormals->width = win::internalWidth;
		texNormals->height = win::internalHeight;
		texNormals->internalFormat = rn::format::RGB32F.layout;
		texNormals->reload();

		fbZPrefill.attachColor(1, texNormals);
	}

	{
		auto texDepth = make_shared<rn::Tex2D>("App::fbZPrefill.depth");
		texDepth->width = win::internalWidth;
		texDepth->height = win::internalHeight;
		texDepth->internalFormat = rn::format::D32F.layout;
		texDepth->reload();

		fbZPrefill.attachDepth(texDepth);
	}

	fbZPrefill.clearColor = glm::vec4{0.f, 0.f, 0.f, 0.f};
	fbZPrefill.reload();

	// fbScreenForward
	{
		auto texColor = make_shared<rn::Tex2D>("App::fbScreenForward.color[0]");
		texColor->width = win::internalWidth;
		texColor->height = win::internalHeight;
		texColor->internalFormat = rn::format::RGBA16F.layout;
		texColor->reload();

		fbScreenForward.attachColor(0, texColor);
	}

	{
		auto texDepth = static_pointer_cast<rn::Tex2D>(fbZPrefill.shareDepth());
		fbScreenForward.attachDepth(texDepth);
	}

	fbScreenForward.clearColor = glm::vec4{0.f, 0.f, 0.f, 0.f};
	fbScreenForward.reload();

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
	// {
	// 	auto texDebug = make_shared<rn::Tex2D>("App::fbShadowMap.color[0]");
	// 	texDebug->width = 1024;
	// 	texDebug->height = 1024;
	// 	texDebug->wrapS = rn::WRAP_BORDER;
	// 	texDebug->wrapT = rn::WRAP_BORDER;
	// 	texDebug->borderColor = glm::vec4{1.f};
	// 	texDebug->internalFormat = rn::format::RGB32F.layout;
	// 	texDebug->reload();

	// 	fbShadowMap.attachColor(0, texDebug);

	// 	auto texDepth = make_shared<rn::Tex2D>("App::fbShadowMap.depth");
	// 	texDepth->width = 1024;
	// 	texDepth->height = 1024;
	// 	texDepth->wrapS = rn::WRAP_BORDER;
	// 	texDepth->wrapT = rn::WRAP_BORDER;
	// 	texDepth->borderColor = glm::vec4{1.f};
	// 	texDepth->internalFormat = rn::format::D32F.layout;
	// 	texDepth->reload();

	// 	fbShadowMap.attachDepth(texDepth);
	// }

	// fbShadowMap.clearColor = glm::vec4{1.f};
	// fbShadowMap.reload();

	// fbShadowMapBlur
	// {
	// 	auto texDebug = make_shared<rn::Tex2D>("App::fbShadowMapBlur.color[0]");
	// 	texDebug->width = 1024;
	// 	texDebug->height = 1024;
	// 	texDebug->internalFormat = rn::format::RGB32F.layout;
	// 	texDebug->reload();

	// 	fbShadowMapBlur.attachColor(0, texDebug);
	// }

	// fbShadowMapBlur.reload();

	directionalLightData.reload();
	pointLightData.reload();
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
		position.z = -10.f;

		auto &rotation = ecs::get<Rotation>(cameraId).rotation;
		rotation.x = 0.f;
		rotation.y = 0.f;
		rotation.z = 0.f;

		auto &projection = ecs::get<Projection>(cameraId);
		projection.zNear = 0.25f;
		projection.zFar = numeric_limits<float>::infinity();

		auto resizeCallback = [&] ()
		{
			auto &projection = ecs::get<Projection>(cameraId);

			projection.aspect = static_cast<float>(win::internalWidth) / static_cast<float>(win::internalHeight);
			// projection.matrix = glm::perspective(glm::radians(projection.fovy), projection.aspect, projection.zNear, (projection.zFar = projection.zNear + 512.0));
			projection.matrix = glm::infinitePerspective(glm::radians(projection.fovy), projection.aspect, projection.zNear);
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
		position.z = 4.f;

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
		glm::vec3 translate{0.f};
		glm::vec3 rotate{0.f};

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
		translate.z = (key::pressed('w') - key::pressed('s'));

		if (translate.x != 0.f || translate.z != 0.f)
		{
			glm::vec2 planar = glm::normalize(glm::vec2{translate.x, translate.z});

			translate.x = planar.x;
			translate.z = planar.y;
		}

		rotate.y = (key::pressed(KEY_RIGHT) - key::pressed(KEY_LEFT));
		rotate.x = (key::pressed(KEY_DOWN) - key::pressed(KEY_UP));

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

	if (key::hit('-'))
	{
		toggleDeferred.change();
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
	RN_CHECK(glClearColor(0.f, 0.f, 0.f, 1.f));
	RN_CHECK(glClearDepth(0.0));
	RN_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

	if ( ! toggleDeferred.value)
	{
		profRender.start();

		profZPrefill.start();
		zPrefillForwardPass();
		profZPrefill.stop();

		profSetupLights.start();
		setupLightsForwardPass();
		profSetupLights.stop();

		profLighting.start();
		lightingForwardPass();
		profLighting.stop();

		{
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);

			progFBBlit.use();
			progFBBlit.uniform("texSource", fbScreenForward.color(0)->bind(0));

			rn::Mesh::quad.render();

			progFBBlit.forgo();
		}

		// if (false)
		{
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);

			progZDebug.use();
			progZDebug.uniform("texNormal", fbZPrefill.color(0)->bind(0));
			progZDebug.uniform("texDebug", fbZPrefill.color(1)->bind(1));
			progZDebug.uniform("texScreen", fbScreenForward.color(0)->bind(2));

			rn::Mesh::quad.render();

			progZDebug.forgo();
		}

		profRender.stop();

		{
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);
			RN_SCOPE_DISABLE(GL_CULL_FACE);
			RN_SCOPE_ENABLE(GL_BLEND);
			// RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
			RN_CHECK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE));

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

			oss << "render=" << profRender.ms() << "ms (" << (1000.0 / profRender.ms()) << ")\n";
			oss << "  ZPrefill=" << profZPrefill.ms() << "ms\n";
			oss << "  SetupLights=" << profSetupLights.ms() << "ms\n";
			oss << "  Lighting=" << profLighting.ms() << "ms\n";

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
			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

			progFBBlit.use();
			progFBBlit.uniform("texSource", fbUI.color(0)->bind(0));
			rn::Mesh::quad.render();
			progFBBlit.forgo();

			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		}
	}
	else
	{
		profRender.start();

		fbScreen.bind();
		fbScreen.clear(rn::BUFFER_COLOR | rn::BUFFER_DEPTH | rn::BUFFER_STENCIL);

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
			RN_SCOPE_DISABLE(GL_BLEND);
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);
			RN_SCOPE_DISABLE(GL_STENCIL_TEST);

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

		{
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);
			RN_SCOPE_DISABLE(GL_CULL_FACE);
			RN_SCOPE_ENABLE(GL_BLEND);
			// RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
			RN_CHECK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE));

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
			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

			progFBBlit.use();
			progFBBlit.uniform("texSource", fbUI.color(0)->bind(0));
			rn::Mesh::quad.render();
			progFBBlit.forgo();

			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		}
	}
}

void App::zPrefillForwardPass()
{
	const auto &projection = ecs::get<Projection>(cameraId);
	const auto &P = projection.matrix;
	const auto &view = ecs::get<View>(cameraId);
	const auto &V = view.matrix;

	const phs::Frustum frustum{P * V};

	// RN_SCOPE_DISABLE(GL_BLEND);

	RN_FB_BIND(fbZPrefill);

	// fbZPrefill.bind();
	fbZPrefill.clear(rn::BUFFER_COLOR | rn::BUFFER_DEPTH | rn::BUFFER_STENCIL);

	progZPrefill.use();
	progZPrefill.uniform("P", P);
	progZPrefill.uniform("V", V);

	GLint locationM = progZPrefill.getName("M");

	for (auto &entity : ecs::findWith<Transform, Mesh, Material>())
	{
		if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
		{
			continue;
		}

		proc::MeshRenderer::renderZ(entity, progZPrefill, locationM);
	}

	progZPrefill.forgo();

	// fbZPrefill.unbind();
}

void App::setupLightsForwardPass()
{
	const auto &view = ecs::get<View>(cameraId);
	const auto &V = view.matrix;

	pointLightData.clear();

	directionalLightCount = 0;
	for (auto &entity : ecs::findWith<PointLight, Position>())
	{
		const auto &light = ecs::get<PointLight>(entity);
		glm::vec3 position{V * glm::vec4{ecs::get<Position>(entity).position, 1.0f}};

		pointLightData.appendData(light.color, light.linearAttenuation, light.quadraticAttenuation, position);
		directionalLightCount++;
	}

	pointLightData.upload();

	directionalLightData.clear();

	pointLightCount = 0;
	for (auto &entity : ecs::findWith<DirectionalLight>())
	{
		const auto &light = ecs::get<DirectionalLight>(entity);
		glm::vec3 direction{V * glm::vec4{light.direction, 0.0f}};
		direction = glm::normalize(direction);

		directionalLightData.appendData(light.ambient, light.color, direction, light.intensity);
		pointLightCount++;
	}

	directionalLightData.upload();
}

void App::lightingForwardPass()
{
	const auto &projection = ecs::get<Projection>(cameraId);
	const auto &P = projection.matrix;
	const auto &view = ecs::get<View>(cameraId);
	const auto &V = view.matrix;

	const phs::Frustum frustum{P * V};

	RN_FB_BIND(fbScreenForward);

	fbScreenForward.clear(rn::BUFFER_COLOR);

	RN_CHECK(glDepthMask(GL_FALSE));
	// RN_SCOPE_DISABLE(GL_DEPTH_TEST);

	progLightingForward.use();
	progLightingForward.uniform("P", P);
	progLightingForward.uniform("V", V);

	progLightingForward.uniform("lightD", directionalLightData.bind(0));
	progLightingForward.uniform("lightP", pointLightData.bind(1));
	progLightingForward.uniform("lightDCount", directionalLightCount);
	progLightingForward.uniform("lightPCount", pointLightCount);

	for (auto &entity : ecs::findWith<Transform, Mesh, Material>())
	{
		if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
		{
			continue;
		}

		proc::MeshRenderer::render(entity, progLightingForward);
	}

	progLightingForward.forgo();

	RN_CHECK(glDepthMask(GL_TRUE));
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

			// progDirectionalLight.var("texShadowMoments", fbShadowMap.color(0)->bind(unit++));
			// progDirectionalLight.var("texShadowDepth", fbShadowMap.depth()->bind(unit++));
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

	if ( ! toggleSSAO.value)
	{
		return;
	}

	ssao.genMipMaps(fbGBuffer);
	ssao.computeAO(fbGBuffer);

	if (toggleSSAOBlur.value) {
		ssao.blur(fbGBuffer);
	}
}
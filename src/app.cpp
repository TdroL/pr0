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
#include <app/comp/shading.hpp>
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

struct AppVariables
{
	ecs::Entity cameraId{};
	ecs::Entity lightIds[10]{};

	rn::Font font1{"DejaVuSansMono"};
	rn::Font font2{"DejaVuSansMono"};

	rn::Program progZPrefill{};
	rn::Program progZDebug{};
	rn::Program progLightingForward{};
	rn::FB fbZPrefill{"App::fbZPrefill"};
	rn::FB fbScreenForward{"App::fbScreenForward"};
	rn::Prof profZPrefill{"App::profZPrefill"};
	rn::Prof profSetupLights{"App::profSetupLights"};
	rn::Prof profRenderShadows{"App::profRenderShadows"};
	rn::Prof profLighting{"App::profLighting"};
	rn::TB directionalLightData{"App::directionalLightData"};
	rn::TB pointLightData{"App::pointLightData"};
	int directionalLightCount = 0;
	int pointLightCount = 0;

	rn::Program progGBuffer{};
	// rn::Program progShadowMap{};
	rn::Program progDirectionalLight{};
	rn::Program progPointLight{};
	rn::Program progFlatLight{};
	rn::Program progSSAOBlit{};
	rn::Program progFBBlit{};
	// rn::Program progBlurGaussian7{};
	rn::Program progBlurPreview{};
	rn::Program progShadowMapPreview{};
	rn::Program progTexPreview{};

	rn::FB fbGBuffer{"App::fbGBuffer"};
	rn::FB fbScreen{"App::fbScreen"};
	rn::FB fbUI{"App::fbUI"};
	// rn::FB fbShadowMap{"App::fbShadowMap"};
	// rn::FB fbShadowMapBlur{"App::fbShadowMapBlur"};

	fx::SSAO ssao{};
	fx::CSM csm{};

	rn::Prof profRender{"App::profRender"};
	rn::Prof profGBuffer{"App::profGBuffer"};
	rn::Prof profDirectionalLight{"App::profDirectionalLight"};
	rn::Prof profPointLight{"App::profPointLight"};
	rn::Prof profFlatLight{"App::profFlatLight"};
	rn::Prof profSSAO{"App::profSSAO"};

	app::Scene scene{};

	util::Timer sunTimer{};

	util::Toggle toggleDeferred{"Deferred (-)", 0};
	util::Toggle toggleSSAO{"SSAO (b)", 0};
	util::Toggle toggleSSAOBlur{"SSAOBlur (n)", 1};
	util::Toggle toggleDebugPreview{"DebugPreview (m)", 0};
	util::Toggle toggleZPreview{"ZPreview (,)", 0};
	util::Toggle toggleLights{"Lights (;)", 1};
	util::Toggle toggleColor{"Color (/)", 0};
	util::Toggle toggleCascade{"Cascade (')", 0, 4};
	util::Toggle toggleCalculateMatrices{"CalculateMatrices ([)", 1};

	const win::Mode modes[3]
	{
		win::Mode::windowed,
		win::Mode::borderless,
		win::Mode::fullscreen
	};

	const string modeNames[3]
	{
		"windowed",
		"borderless",
		"fullscreen",
	};

	size_t currentMode = 0;

	const int vsyncs[3]
	{
		-1, // progressive
		 0, // off
		 1, // on
	};

	const string vsyncNames[3]
	{
		"progressive",
		"off",
		"on",
	};

	size_t currentVsync = 0;
};

App::App()
{
	v = new AppVariables{};
}

App::~App()
{
	delete v;
}

void App::init()
{
	v->font1.load("DejaVu/DejaVuSansMono.ttf");
	v->font2.load("DejaVu/DejaVuSansMono.ttf");

	clog << "Init shader programs:" << endl;
	initProg();

	clog << "Init frame buffers:" << endl;
	initFB();

	clog << "Init scene objects:" << endl;
	initScene();

	v->ssao.init(ecs::get<Projection>(v->cameraId));
	v->csm.init();

	// v->profZPrefill
	// v->profSetupLights
	// v->profLighting
	// v->profRender
	// v->profGBuffer
	// v->profDirectionalLight
	// v->profPointLight
	// v->profFlatLight
	// v->profSSAO

	v->profZPrefill.init();
	v->profSetupLights.init();
	v->profRenderShadows.init();
	v->profLighting.init();

	v->profRender.init();
	v->profGBuffer.init();
	v->profDirectionalLight.init();
	v->profPointLight.init();
	v->profFlatLight.init();
	v->profSSAO.init();

	/* Test: switch to window mode */
	UTIL_DEBUG
	{
		v->currentMode = 0;
		v->currentVsync = 1;

		clog << "Test: switching to " << v->modeNames[v->currentMode] << " " << v->vsyncNames[v->currentVsync] << endl;

		win::switchMode(v->modes[v->currentMode], v->vsyncs[v->currentVsync]);
		rn::reloadSoftAll();
	}
}

void App::initProg()
{
	try
	{
		v->progFBBlit.load("rn/fboBlit.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		v->progBlurPreview.load("rn/blurPreview.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		v->progShadowMapPreview.load("lighting/shadows/preview.frag", "rn/fboM.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		v->progTexPreview.load("rn/fbo.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		v->progGBuffer.load("lighting/deferred/gbuffer.frag", "PN.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		v->progZPrefill.load("lighting/forward/zPrefill.frag", "lighting/forward/zPrefill.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		v->progZDebug.load("lighting/forward/zDebug.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		v->progLightingForward.load("lighting/forward/lighting.frag", "lighting/forward/lighting.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	// try
	// {
	// 	v->progBlurGaussian7.load("rn/blurGaussian7.frag", "rn/fbo.vert");
	// }
	// catch (const string &e)
	// {
	// 	cerr << "Warning: " << e << endl;
	// }

	// try
	// {
	// 	v->progShadowMap.load("lighting/shadows/depthVSM.frag", "P.vert");
	// }
	// catch (const string &e)
	// {
	// 	cerr << "Warning: " << e << endl;
	// }

	try
	{
		v->progDirectionalLight.load("lighting/deferred/directionallight.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		v->progPointLight.load("lighting/deferred/pointlight.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		v->progFlatLight.load("lighting/deferred/flatlight.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		v->progSSAOBlit.load("lighting/ssaoBlit.frag", "rn/fbo.vert");
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

		v->fbGBuffer.attachColor(0, texMaterial);
	}

	{
		auto texNormals = make_shared<rn::Tex2D>("App::fbGBuffer.color[1]");

		texNormals->width = win::internalWidth;
		texNormals->height = win::internalHeight;
		texNormals->internalFormat = rn::format::RG16F.layout;
		texNormals->reload();

		v->fbGBuffer.attachColor(1, texNormals);
	}

	{
		auto texZ = make_shared<rn::Tex2D>("App::fbGBuffer.color[2]");
		texZ->width = win::internalWidth;
		texZ->height = win::internalHeight;
		// texZ->internalFormat = rn::format::R32F.layout;
		texZ->internalFormat = rn::format::RGB32F.layout;
		texZ->reload();

		v->fbGBuffer.attachColor(2, texZ);
	}

	{
		auto texDepth = make_shared<rn::Tex2D>("App::fbGBuffer.depth");
		texDepth->width = win::internalWidth;
		texDepth->height = win::internalHeight;
		texDepth->internalFormat = rn::format::D32FS8.layout;
		texDepth->reload();

		v->fbGBuffer.attachDepth(texDepth);
	}

	v->fbGBuffer.reload();

	// fbScreen
	{
		auto texColor = make_shared<rn::Tex2D>("App::fbScreen.color[0]");
		texColor->width = win::internalWidth;
		texColor->height = win::internalHeight;
		texColor->internalFormat = rn::format::RGBA16F.layout;
		texColor->reload();

		v->fbScreen.attachColor(0, texColor);
	}

	{
		auto texDepth = make_shared<rn::Tex2D>("App::fbScreen.depth");
		texDepth->width = win::internalWidth;
		texDepth->height = win::internalHeight;
		texDepth->internalFormat = rn::format::D32FS8.layout;
		texDepth->reload();

		v->fbScreen.attachDepth(texDepth);
	}

	v->fbScreen.clearColorValue = glm::vec4{0.f, 0.f, 0.f, 0.f};
	v->fbScreen.reload();

	// fbZPrefill
	{
		auto texNormals = make_shared<rn::Tex2D>("App::fbZPrefill.color[0]");

		texNormals->width = win::internalWidth;
		texNormals->height = win::internalHeight;
		texNormals->internalFormat = rn::format::RG16F.layout;
		texNormals->reload();

		v->fbZPrefill.attachColor(0, texNormals);
	}

	{
		auto texNormals = make_shared<rn::Tex2D>("App::fbZPrefill.color[1]");

		texNormals->width = win::internalWidth;
		texNormals->height = win::internalHeight;
		texNormals->internalFormat = rn::format::RGB32F.layout;
		texNormals->reload();

		v->fbZPrefill.attachColor(1, texNormals);
	}

	{
		auto texDepth = make_shared<rn::Tex2D>("App::fbZPrefill.depth");
		texDepth->width = win::internalWidth;
		texDepth->height = win::internalHeight;
		texDepth->internalFormat = rn::format::D32F.layout;
		texDepth->reload();

		v->fbZPrefill.attachDepth(texDepth);
	}

	v->fbZPrefill.clearColorValue = glm::vec4{0.f, 0.f, 0.f, 0.f};
	v->fbZPrefill.reload();

	// fbScreenForward
	{
		auto texColor = make_shared<rn::Tex2D>("App::fbScreenForward.color[0]");
		texColor->width = win::internalWidth;
		texColor->height = win::internalHeight;
		texColor->internalFormat = rn::format::RGBA16F.layout;
		texColor->reload();

		v->fbScreenForward.attachColor(0, texColor);
	}

	{
		auto texDepth = static_pointer_cast<rn::Tex2D>(v->fbZPrefill.shareDepth());
		v->fbScreenForward.attachDepth(texDepth);
	}

	v->fbScreenForward.clearColorValue = glm::vec4{0.f, 0.f, 0.f, 0.f};
	v->fbScreenForward.reload();

	// fbUI
	{
		auto texColor = make_shared<rn::Tex2D>("App::fbUI.color[0]");
		texColor->width = win::internalWidth;
		texColor->height = win::internalHeight;
		texColor->internalFormat = rn::format::RGBA8.layout;
		texColor->reload();

		v->fbUI.attachColor(0, texColor);
	}

	v->fbUI.clearColorValue = glm::vec4{0.f, 0.f, 0.f, 0.f};
	v->fbUI.reload();

	event::subscribe<win::WindowResizeEvent>([&] (const win::WindowResizeEvent &)
	{
		/*
		auto texMaterial = dynamic_cast<rn::Tex2D *>(v->fbGBuffer.color(0));
		texMaterial->width = win::internalWidth;
		texMaterial->height = win::internalHeight;

		auto texNormals = dynamic_cast<rn::Tex2D *>(v->fbGBuffer.color(1));
		texNormals->width = win::internalWidth;
		texNormals->height = win::internalHeight;

		auto texZ = dynamic_cast<rn::Tex2D *>(v->fbGBuffer.color(2));
		texZ->width = win::internalWidth;
		texZ->height = win::internalHeight;

		auto texDepth = dynamic_cast<rn::Tex2D *>(v->fbGBuffer.depth());
		texDepth->width = win::internalWidth;
		texDepth->height = win::internalHeight;

		v->fbGBuffer.width = win::internalWidth;
		v->fbGBuffer.height = win::internalHeight;
		v->fbGBuffer.reload();

		auto texDepth = dynamic_cast<rn::Tex2D *>(v->fbScreen.color(0));
		texColor->width = win::internalWidth;
		texColor->height = win::internalHeight;

		auto texDepth = dynamic_cast<rn::Tex2D *>(v->fbScreen.depth());
		texDepth->width = win::internalWidth;
		texDepth->height = win::internalHeight;

		v->fbScreen.width = win::internalWidth;
		v->fbScreen.height = win::internalHeight;
		v->fbScreen.reload();
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

	// 	v->fbShadowMap.attachColor(0, texDebug);

	// 	auto texDepth = make_shared<rn::Tex2D>("App::fbShadowMap.depth");
	// 	texDepth->width = 1024;
	// 	texDepth->height = 1024;
	// 	texDepth->wrapS = rn::WRAP_BORDER;
	// 	texDepth->wrapT = rn::WRAP_BORDER;
	// 	texDepth->borderColor = glm::vec4{1.f};
	// 	texDepth->internalFormat = rn::format::D32F.layout;
	// 	texDepth->reload();

	// 	v->fbShadowMap.attachDepth(texDepth);
	// }

	// v->fbShadowMap.clearColorValue = glm::vec4{1.f};
	// v->fbShadowMap.reload();

	// fbShadowMapBlur
	// {
	// 	auto texDebug = make_shared<rn::Tex2D>("App::fbShadowMapBlur.color[0]");
	// 	texDebug->width = 1024;
	// 	texDebug->height = 1024;
	// 	texDebug->internalFormat = rn::format::RGB32F.layout;
	// 	texDebug->reload();

	// 	v->fbShadowMapBlur.attachColor(0, texDebug);
	// }

	// v->fbShadowMapBlur.reload();

	v->directionalLightData.reload();
	v->pointLightData.reload();
}

void App::initScene()
{
	try
	{
		v->scene.reload();
	}
	catch (const string &e)
	{
		cerr << e << endl;
	}

	/* Create camera */
	{
		v->cameraId = ecs::create();

		ecs::enable<Name, Position, Rotation, Projection, View>(v->cameraId);

		auto &name = ecs::get<Name>(v->cameraId);
		name.name = "Camera";

		auto &position = ecs::get<Position>(v->cameraId).position;
		position.x = 0.f;
		position.y = 0.125f;
		position.z = -10.f;

		auto &rotation = ecs::get<Rotation>(v->cameraId).rotation;
		rotation.x = 0.f;
		rotation.y = 0.f;
		rotation.z = 0.f;

		auto &projection = ecs::get<Projection>(v->cameraId);
		projection.zNear = 0.25f;
		projection.zFar = numeric_limits<float>::infinity();

		auto resizeCallback = [&] ()
		{
			auto &projection = ecs::get<Projection>(v->cameraId);

			projection.aspect = static_cast<float>(win::internalWidth) / static_cast<float>(win::internalHeight);
			// projection.matrix = glm::perspective(glm::radians(projection.fovy), projection.aspect, projection.zNear, (projection.zFar = projection.zNear + 512.0));
			// projection.matrix = glm::infinitePerspective(glm::radians(projection.fovy), projection.aspect, projection.zNear);
			projection.matrix = glm::infiniteReversePerspective(glm::radians(projection.fovy), projection.aspect, projection.zNear);
			projection.invMatrix = glm::inverse(projection.matrix);

			v->progGBuffer.uniform("P", projection.matrix);

			v->progPointLight.uniform("invP", projection.invMatrix);
			v->progDirectionalLight.uniform("invP", projection.invMatrix);
			v->progSSAOBlit.uniform("invP", projection.invMatrix);

			glm::mat4 previewM{1.f};
			previewM = glm::translate(previewM, glm::vec3{0.75f, 0.75f, 0.f});
			previewM = glm::scale(previewM, glm::vec3{0.25f / projection.aspect, 0.25f, 0.f});
			// previewM = glm::scale(previewM, glm::vec3{0.25f});

			v->progShadowMapPreview.uniform("M", previewM);
			v->progBlurPreview.uniform("M", previewM);
		};

		resizeCallback();

		event::subscribe<win::WindowResizeEvent>([&resizeCallback] (const win::WindowResizeEvent &)
		{
			resizeCallback();
		});
	}

	// create lights
	v->lightIds[0] = ecs::create();
	v->lightIds[1] = ecs::create();
	v->lightIds[2] = ecs::create();
	v->lightIds[3] = ecs::create();

	// directional light
	v->lightIds[4] = ecs::create();

	// light #1
	{
		ecs::enable<Name, PointLight, Position, Transform, Mesh>(v->lightIds[0]);

		auto &name = ecs::get<Name>(v->lightIds[0]);
		name.name = "PointLight #1";

		auto &light = ecs::get<PointLight>(v->lightIds[0]);
		light.color = glm::vec4{1.f, 1.f, 1.f, 1.f};
		// linear attenuation; distance at which half of the light intensity is lost
		light.linearAttenuation = 1.0 / pow(3.0, 2); // r_l = 3.0;
		// quadriatic attenuation; distance at which three-quarters of the light intensity is lost
		light.quadraticAttenuation = 1.0 / pow(6.0, 2); // r_q = 6.0;

		auto &position = ecs::get<Position>(v->lightIds[0]).position;
		position.x = 0.f;
		position.y = 1.f;
		position.z = 0.f;

		auto &transform = ecs::get<Transform>(v->lightIds[0]);
		transform.translation.x = position.x;
		transform.translation.y = position.y;
		transform.translation.z = position.z;
		transform.scale = glm::vec3{0.25f};

		ecs::get<Mesh>(v->lightIds[0]).id = asset::mesh::load("sphere.sbm");
	}

	// light #2
	{
		ecs::enable<Name, PointLight, Position, Transform, Mesh>(v->lightIds[1]);

		auto &name = ecs::get<Name>(v->lightIds[1]);
		name.name = "PointLight #2";

		auto &light = ecs::get<PointLight>(v->lightIds[1]);
		light.color = glm::vec4{1.f, 1.f, 1.f, 1.f};
		// linear attenuation; distance at which half of the light intensity is lost
		light.linearAttenuation = 1.0 / pow(0.5, 2); // r_l = 3.0;
		// quadriatic attenuation; distance at which three-quarters of the light intensity is lost
		light.quadraticAttenuation = 1.0 / pow(1.0, 2); // r_q = 6.0;

		auto &position = ecs::get<Position>(v->lightIds[1]).position;
		position.x = 0.f;
		position.y = 1.f;
		position.z = 0.f;

		auto &transform = ecs::get<Transform>(v->lightIds[1]);
		transform.translation.x = position.x;
		transform.translation.y = position.y;
		transform.translation.z = position.z;
		transform.scale = glm::vec3{0.25f};

		ecs::get<Mesh>(v->lightIds[1]).id = asset::mesh::load("sphere.sbm");
	}

	// light #3
	{
		ecs::enable<Name, PointLight, Position, Transform, Mesh>(v->lightIds[2]);

		auto &name = ecs::get<Name>(v->lightIds[2]);
		name.name = "PointLight Red";

		auto &light = ecs::get<PointLight>(v->lightIds[2]);
		light.color = glm::vec4{1.f, 0.f, 0.f, 1.f};
		// linear attenuation; distance at which half of the light intensity is lost
		light.linearAttenuation = 1.0 / pow(1.5, 2); // r_l = 3.0;
		// quadriatic attenuation; distance at which three-quarters of the light intensity is lost
		light.quadraticAttenuation = 1.0 / pow(4.0, 2); // r_q = 6.0;

		auto &position = ecs::get<Position>(v->lightIds[2]).position;
		position.x = 2.f;
		position.y = 0.5f;
		position.z = 4.f;

		auto &transform = ecs::get<Transform>(v->lightIds[2]);
		transform.translation.x = position.x;
		transform.translation.y = position.y;
		transform.translation.z = position.z;
		transform.scale = glm::vec3{0.25f};

		ecs::get<Mesh>(v->lightIds[2]).id = asset::mesh::load("sphere.sbm");
	}

	// light #4
	{
		ecs::enable<Name, PointLight, Position, Transform, Mesh>(v->lightIds[3]);

		auto &name = ecs::get<Name>(v->lightIds[3]);
		name.name = "PointLight Green";

		auto &light = ecs::get<PointLight>(v->lightIds[3]);
		light.color = glm::vec4{0.f, 1.f, 0.f, 1.f};
		// linear attenuation; distance at which half of the light intensity is lost
		light.linearAttenuation = 1.0 / pow(1.5, 2); // r_l = 3.0;
		// quadriatic attenuation; distance at which three-quarters of the light intensity is lost
		light.quadraticAttenuation = 1.0 / pow(4.0, 2); // r_q = 6.0;

		auto &position = ecs::get<Position>(v->lightIds[3]).position;
		position.x = -3.f;
		position.y = 0.25f;
		position.z = -0.35f;

		auto &transform = ecs::get<Transform>(v->lightIds[3]);
		transform.translation.x = position.x;
		transform.translation.y = position.y;
		transform.translation.z = position.z;
		transform.scale = glm::vec3{0.25f};

		ecs::get<Mesh>(v->lightIds[3]).id = asset::mesh::load("sphere.sbm");
	}

	// directional light #1
	{
		ecs::enable<Name, DirectionalLight, Transform, Mesh>(v->lightIds[4]);

		auto &name = ecs::get<Name>(v->lightIds[4]);
		name.name = "DirectionalLight";

		auto &light = ecs::get<DirectionalLight>(v->lightIds[4]);
		light.ambient = glm::vec4{0.f, 0.36f, 0.5f, 1.f};
		light.color = glm::vec4{0.8f, 0.8f, 0.8f, 1.f};
		light.direction = glm::vec3{0.5f, 0.5f, 1.f};
		light.intensity = 4.f;

		auto &transform = ecs::get<Transform>(v->lightIds[4]);
		transform.translation = light.direction * 100.f;
		transform.scale = glm::vec3{10.f};

		ecs::get<Mesh>(v->lightIds[4]).id = asset::mesh::load("sphere.sbm");
	}

	for (auto &entity : ecs::findWith<Name, Transform, Mesh>())
	{
		proc::RebuildBoundingObjectProcess::update(entity);
	}

	{
		glm::vec3 translate{0.f};
		glm::vec3 rotate{0.f};

		proc::Camera::update(v->cameraId, translate, rotate);
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

		proc::Camera::update(v->cameraId, translate, rotate);
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
			v->scene.reload();
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
		v->currentVsync = (v->currentVsync + 1) % util::countOf(v->vsyncs);

		cout << "Switching vsync mode to \"" << v->vsyncNames[v->currentVsync] << "\" (" << v->vsyncs[v->currentVsync] << ") ..." << endl;
		win::switchMode(v->modes[v->currentMode], v->vsyncs[v->currentVsync]);
		rn::reloadSoftAll();
		cout << "done" << endl;
	}

	if (key::hit(KEY_F11))
	{
		cout << "Switching window mode..." << endl;
		v->currentMode = (v->currentMode + 1) % util::countOf(v->modes);
		win::switchMode(v->modes[v->currentMode], v->vsyncs[v->currentVsync]);
		cout << "done" << endl;

		cout << "Soft reloading GL..." << endl;
		rn::reloadSoftAll();
		cout << "done" << endl;
	}

	if (key::hit('t'))
	{
		v->sunTimer.togglePause();
	}

	if (key::hit('-'))
	{
		v->toggleDeferred.change();
	}

	if (key::hit('b'))
	{
		v->toggleSSAO.change();
	}

	if (key::hit('n'))
	{
		v->toggleSSAOBlur.change();
	}

	if (key::hit('m'))
	{
		v->toggleDebugPreview.change();
	}

	if (key::hit(','))
	{
		v->toggleZPreview.change();
	}

	if (key::hit(';'))
	{
		v->toggleLights.change();
	}

	if (key::hit('/'))
	{
		v->toggleColor.change();
	}

	if (key::hit('\''))
	{
		v->toggleCascade.change();
	}

	if (key::hit('['))
	{
		v->toggleCalculateMatrices.change();
	}

	{
		glm::vec3 lightPositionDelta{static_cast<float>(5.0 * ngn::dt)};

		lightPositionDelta.x *= (ngn::key::pressed(KEY_KP_6)   - ngn::key::pressed(KEY_KP_4));
		lightPositionDelta.y *= (ngn::key::pressed(KEY_KP_ADD) - ngn::key::pressed(KEY_KP_ENTER));
		lightPositionDelta.z *= (ngn::key::pressed(KEY_KP_5)   - ngn::key::pressed(KEY_KP_8));

		auto &position = ecs::get<Position>(v->lightIds[0]).position;
		position += lightPositionDelta;

		auto &transform = ecs::get<Transform>(v->lightIds[0]);
		transform.translation = position;
	}

	{
		glm::vec3 lightPositionDelta{3.f, 1.f, 3.f};

		auto &position = ecs::get<Position>(v->lightIds[1]).position;
		position.x = lightPositionDelta.x * sin(ngn::ct * 0.5f);
		position.y = lightPositionDelta.y * sin(ngn::ct);
		position.z = lightPositionDelta.z * cos(ngn::ct * 0.5f);

		auto &transform = ecs::get<Transform>(v->lightIds[1]);
		transform.translation = position;
	}

	{
		glm::vec3 lightPositionDelta{1.f, 0.25f, 1.f};

		auto &directionalLight = ecs::get<DirectionalLight>(v->lightIds[4]);
		directionalLight.direction.x = lightPositionDelta.x * sin(v->sunTimer.timed); //  + 3.151592f / 4.f
		directionalLight.direction.z = lightPositionDelta.z * cos(v->sunTimer.timed);

		auto &transform = ecs::get<Transform>(v->lightIds[4]);
		const auto &camPosition = ecs::get<Position>(v->cameraId).position;
		transform.translation = camPosition + directionalLight.direction * 100.f;
	}

	v->sunTimer.update();

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

	if ( ! v->toggleDeferred.value)
	{
		v->profRender.start();

		v->profZPrefill.start();
		zPrefillForwardPass();
		v->profZPrefill.stop();

		v->profSSAO.start();
		ssaoPass();
		v->profSSAO.stop();

		v->profSetupLights.start();
		setupLightsForwardPass();
		v->profSetupLights.stop();

		v->profRenderShadows.start();
		renderShadowsForwardPass();
		v->profRenderShadows.stop();

		v->profLighting.start();
		lightingForwardPass();
		v->profLighting.stop();

		{
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);

			v->progFBBlit.use();
			v->progFBBlit.uniform("texSource", v->fbScreenForward.color(0)->bind(0));

			rn::Mesh::quad.render();

			v->progFBBlit.forgo();
		}

		// if (false)
		{
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);

			v->progZDebug.use();
			v->progZDebug.uniform("texNormal", v->fbZPrefill.color(0)->bind(0));
			v->progZDebug.uniform("texDebug", v->fbZPrefill.color(1)->bind(1));
			v->progZDebug.uniform("texScreen", v->fbScreenForward.color(0)->bind(2));

			rn::Mesh::quad.render();

			v->progZDebug.forgo();
		}

		v->profRender.stop();

		{
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);
			RN_SCOPE_DISABLE(GL_CULL_FACE);
			RN_SCOPE_ENABLE(GL_BLEND);
			// RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
			RN_CHECK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE));

			double ft = ngn::time() - ngn::ct;

			v->fbUI.bind();
			v->fbUI.clear(rn::BUFFER_COLOR);

			ostringstream oss;
			oss << setprecision(4) << fixed;
			oss << "dt=" << ngn::dt * 1000.0 << "ms\n";
			oss << "ft=" << ft * 1000.0 << "ms\n";
			oss << "fps=" << 1.0 / ngn::dt << "\n";
			oss << "fps=" << 1.0 / ft << " (frame)\n";
			oss << "triangles=" << rn::stats.triangles << "\n";
			oss << "\n";
			oss << "render=" << v->profRender.ms() << "ms/" << v->profRender.latency() << "f (" << (1000.0 / v->profRender.ms()) << ")\n";
			oss << "  ZPrefill=" << v->profZPrefill.ms() << "ms/" << v->profZPrefill.latency() << "f\n";
			oss << "  SSAO=" << v->profSSAO.ms() << "ms/" << v->profSSAO.latency() << "f\n";
			oss << "    Z=" << v->ssao.profZ.ms() << "ms/" << v->ssao.profZ.latency() << "f\n";
			oss << "    MipMaps=" << v->ssao.profMipMaps.ms() << "ms/" << v->ssao.profMipMaps.latency() << "f\n";
			oss << "    AO=" << v->ssao.profAO.ms() << "ms/" << v->ssao.profAO.latency() << "f\n";
			oss << "    Blur=" << v->ssao.profBlur.ms() << "ms/" << v->ssao.profBlur.latency() << "f\n";
			oss << "  SetupLights=" << v->profSetupLights.ms() << "ms/" << v->profSetupLights.latency() << "f\n";
			oss << "  RenderShadows=" << v->profRenderShadows.ms() << "ms/" << v->profRenderShadows.latency() << "f\n";
			oss << "  Lighting=" << v->profLighting.ms() << "ms/" << v->profLighting.latency() << "f\n";
			oss << "\n";
			oss << "F4 - reload scene\n";
			oss << "F5 - reload shaders\n";
			oss << "F6 - reload meshes\n";
			oss << "F7 - reload fonts\n";
			oss << "F8 - reload FBOs\n";
			oss << "F9 - reload textures\n";
			oss << "F10 - change vsync mode (current: " << v->vsyncNames[v->currentVsync] << ")\n";
			oss << "F11 - change window mode (current: " << v->modeNames[v->currentMode] << ")\n";
			oss << "\n";
			oss << "Movement: W, A, S, D, SPACE, CTRL (SHIFT - slower movement)\n";
			oss << "Camera: arrows (SHIFT - slower rotation)\n";
			oss << "Point Light: Keypad 8, 4, 5, 6\n\n";
			oss << "Toggles:\n";

			for (auto &toggle : util::Toggle::collection)
			{
				oss << "  " << toggle->toggleName << " = " << toggle->value << "\n";
			}

			v->font1.render(oss.str());

			oss.str(""s);
			oss.clear();

			oss << "CSM:\n";
			oss << "  V[0][0]=" << glm::to_string(v->csm.Vs[0][0]) << "\n";
			oss << "  V[0][1]=" << glm::to_string(v->csm.Vs[0][1]) << "\n";
			oss << "  V[0][2]=" << glm::to_string(v->csm.Vs[0][2]) << "\n";
			oss << "  V[0][3]=" << glm::to_string(v->csm.Vs[0][3]) << "\n";
			oss << "\n";
			oss << "  P[0][0]=" << glm::to_string(v->csm.Ps[0][0]) << "\n";
			oss << "  P[0][1]=" << glm::to_string(v->csm.Ps[0][1]) << "\n";
			oss << "  P[0][2]=" << glm::to_string(v->csm.Ps[0][2]) << "\n";
			oss << "  P[0][3]=" << glm::to_string(v->csm.Ps[0][3]) << "\n";

			v->font2.position.x = -0.125f;
			v->font2.render(oss.str());

			v->fbUI.unbind();

			RN_SCOPE_ENABLE(GL_BLEND);
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);
			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

			v->progFBBlit.use();
			v->progFBBlit.uniform("texSource", v->fbUI.color(0)->bind(0));
			rn::Mesh::quad.render();
			v->progFBBlit.forgo();

			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		}
	}
	else
	{
		v->profRender.start();

		v->fbScreen.bind();
		v->fbScreen.clear(rn::BUFFER_COLOR | rn::BUFFER_DEPTH | rn::BUFFER_STENCIL);

		v->profGBuffer.start();
		gBufferPass();
		v->profGBuffer.stop();

		// v->fbScreen.clear(rn::BUFFER_COLOR);
		v->fbGBuffer.blit(v->fbScreen, rn::BUFFER_STENCIL);
		// v->fbGBuffer.blit(nullptr, rn::BUFFER_STENCIL);

		if ( ! v->toggleLights.value)
		{
			v->fbGBuffer.blit(v->fbScreen, rn::BUFFER_COLOR);
			// v->fbGBuffer.blit(nullptr, rn::BUFFER_COLOR);
		}

		v->profSSAO.start();
		ssaoPass();
		v->profSSAO.stop();

		if (v->toggleLights.value)
		{
			v->profDirectionalLight.start();
			directionalLightsPass();
			v->profDirectionalLight.stop();

			// v->profPointLight.start();
			// pointLightsPass();
			// v->profPointLight.stop();

			v->profFlatLight.start();
			flatLightPass();
			v->profFlatLight.stop();

			// v->fbScreen.blit(nullptr, rn::BUFFER_COLOR/*, rn::MAG_LINEAR*/);
		}

		if (v->toggleDebugPreview.value)
		{
			RN_SCOPE_DISABLE(GL_BLEND);
			RN_SCOPE_ENABLE(GL_STENCIL_TEST);
			RN_CHECK(glStencilFunc(GL_EQUAL, Stencil::MASK_SHADED, Stencil::MASK_ALL));
			RN_CHECK(glStencilMask(0x0));

			v->progSSAOBlit.use();

			v->progSSAOBlit.var("texSource", v->ssao.fbAO.color(0)->bind(0));
			v->progSSAOBlit.var("texColor", v->fbGBuffer.color(0)->bind(1));
			v->progSSAOBlit.var("texNormal", v->fbGBuffer.color(1)->bind(2));
			v->progSSAOBlit.var("texDepth", v->fbGBuffer.depth()->bind(3));
			v->progSSAOBlit.var("texZ", v->ssao.fbZ.color(0)->bind(4));

			rn::Mesh::quad.render();

			v->progSSAOBlit.forgo();
		}

		if (v->toggleZPreview.value)
		{
			RN_SCOPE_DISABLE(GL_BLEND);
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);
			RN_SCOPE_DISABLE(GL_STENCIL_TEST);

			v->progTexPreview.use();
			v->progTexPreview.var("texSource", v->ssao.fbZ.color(0)->bind(0));

			rn::Mesh::quad.render();

			v->progTexPreview.forgo();
		}

		{
			RN_SCOPE_DISABLE(GL_BLEND);
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);
			RN_SCOPE_DISABLE(GL_STENCIL_TEST);

			v->progShadowMapPreview.use();
			// v->progShadowMapPreview.var("texSource", v->fbShadowMap.color(0)->bind(0));
			// v->progShadowMapPreview.var("texSource", v->csm.fbShadows[0].color(0)->bind(0));

			RN_CHECK(glTextureParameteri(v->csm.texDepths->id, GL_TEXTURE_COMPARE_MODE, GL_NONE));

			v->progShadowMapPreview.var("texSource", v->csm.texDepths->bind(0));
			v->progShadowMapPreview.var("layer", v->toggleCascade.value);

			rn::Mesh::quad.render();

			v->progShadowMapPreview.forgo();

			RN_CHECK(glTextureParameteri(v->csm.texDepths->id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
			RN_CHECK_PARAM(glTextureParameteri(v->csm.texDepths->id, GL_TEXTURE_COMPARE_FUNC, v->csm.texDepths->compareFunc), rn::getEnumName(v->csm.texDepths->compareFunc));
		}

		v->fbScreen.unbind();
		v->fbScreen.blit(nullptr, rn::BUFFER_COLOR);

		v->profRender.stop();

		{
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);
			RN_SCOPE_DISABLE(GL_CULL_FACE);
			RN_SCOPE_ENABLE(GL_BLEND);
			// RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
			RN_CHECK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE));

			double ft = ngn::time() - ngn::ct;

			v->fbUI.bind();
			v->fbUI.clear(rn::BUFFER_COLOR);

			ostringstream oss;
			oss << setprecision(4) << fixed;
			oss << "dt=" << ngn::dt * 1000.0 << "ms\n";
			oss << "ft=" << ft * 1000.0 << "ms\n";
			oss << "fps=" << 1.0 / ngn::dt << "\n";
			oss << "fps=" << 1.0 / ft << " (frame)\n";
			oss << "triangles=" << rn::stats.triangles << "\n";
			oss << "\n";

			oss << "render=" << v->profRender.ms() << "ms (" << 1000.0 / v->profRender.ms() << ")\n";
			oss << "  GBuffer=" << v->profGBuffer.ms() << "ms\n";
			oss << "  DirectionalLight=" << v->profDirectionalLight.ms() << "ms\n";
			oss << "  PointLight=" << v->profPointLight.ms() << "ms\n";
			oss << "  FlatLight=" << v->profFlatLight.ms() << "ms\n";
			oss << "  SSAO=" << v->profSSAO.ms() << "ms\n";
			oss << "    Z=" << v->ssao.profZ.ms() << "ms\n";
			oss << "    MipMaps=" << v->ssao.profMipMaps.ms() << "ms\n";
			oss << "    AO=" << v->ssao.profAO.ms() << "ms\n";
			oss << "    Blur=" << v->ssao.profBlur.ms() << "ms\n";
			oss << "  CSM=???ms\n";
			oss << "    Render=" << v->csm.profRender.ms() << "ms\n";
			oss << "    Blur=" << v->csm.profBlur.ms() << "ms\n";

			oss << "\n";
			oss << "F4 - reload scene\n";
			oss << "F5 - reload shaders\n";
			oss << "F6 - reload meshes\n";
			oss << "F7 - reload fonts\n";
			oss << "F8 - reload FBOs\n";
			oss << "F9 - reload textures\n";
			oss << "F10 - change vsync mode (current: " << v->vsyncNames[v->currentVsync] << ")\n";
			oss << "F11 - change window mode (current: " << v->modeNames[v->currentMode] << ")\n";
			oss << "\n";
			oss << "Movement: W, A, S, D\n";
			oss << "Camera: arrows\n";
			oss << "Point Light: Keypad 8, 4, 5, 6\n\n";
			oss << "Toggles:\n";

			for (auto &toggle : util::Toggle::collection)
			{
				oss << "  " << toggle->toggleName << " = " << toggle->value << "\n";
			}

			v->font1.render(oss.str());

			oss.str(""s);
			oss.clear();

			oss << "CSM:\n";
			oss << "  V[0][0]=" << glm::to_string(v->csm.Vs[0][0]) << "\n";
			oss << "  V[0][1]=" << glm::to_string(v->csm.Vs[0][1]) << "\n";
			oss << "  V[0][2]=" << glm::to_string(v->csm.Vs[0][2]) << "\n";
			oss << "  V[0][3]=" << glm::to_string(v->csm.Vs[0][3]) << "\n";
			oss << "\n";
			oss << "  P[0][0]=" << glm::to_string(v->csm.Ps[0][0]) << "\n";
			oss << "  P[0][1]=" << glm::to_string(v->csm.Ps[0][1]) << "\n";
			oss << "  P[0][2]=" << glm::to_string(v->csm.Ps[0][2]) << "\n";
			oss << "  P[0][3]=" << glm::to_string(v->csm.Ps[0][3]) << "\n";

			v->font2.position.x = -0.125f;
			v->font2.render(oss.str());

			v->fbUI.unbind();

			RN_SCOPE_ENABLE(GL_BLEND);
			RN_SCOPE_DISABLE(GL_DEPTH_TEST);
			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

			v->progFBBlit.use();
			v->progFBBlit.uniform("texSource", v->fbUI.color(0)->bind(0));
			rn::Mesh::quad.render();
			v->progFBBlit.forgo();

			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		}
	}
}

void App::zPrefillForwardPass()
{
	const auto &projection = ecs::get<Projection>(v->cameraId);
	const auto &P = projection.matrix;
	const auto &view = ecs::get<View>(v->cameraId);
	const auto &V = view.matrix;

	const phs::Frustum frustum{P * V};

	// RN_SCOPE_DISABLE(GL_BLEND);

	RN_FB_BIND(v->fbZPrefill);

	// v->fbZPrefill.bind();
	v->fbZPrefill.clear(rn::BUFFER_COLOR | rn::BUFFER_DEPTH | rn::BUFFER_STENCIL);

	v->progZPrefill.use();
	v->progZPrefill.uniform("P", P);
	v->progZPrefill.uniform("V", V);

	GLint locationM = v->progZPrefill.getName("M");

	for (auto &entity : ecs::findWith<Transform, Mesh, Material>())
	{
		if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
		{
			continue;
		}

		proc::MeshRenderer::renderZ(entity, v->progZPrefill, locationM);
	}

	v->progZPrefill.forgo();

	// v->fbZPrefill.unbind();
}

void App::setupLightsForwardPass()
{
	const auto &view = ecs::get<View>(v->cameraId);
	const auto &V = view.matrix;

	v->pointLightData.clear();
	v->pointLightCount = 0;
	for (auto &entity : ecs::findWith<PointLight, Position>())
	{
		const auto &light = ecs::get<PointLight>(entity);
		glm::vec3 position{V * glm::vec4{ecs::get<Position>(entity).position, 1.0f}};

		v->pointLightData.appendData(light.color, light.linearAttenuation, light.quadraticAttenuation, position);
		v->pointLightCount++;
	}
	v->pointLightData.upload();

	v->directionalLightData.clear();
	v->directionalLightCount = 0;
	for (auto &entity : ecs::findWith<DirectionalLight>())
	{
		const auto &light = ecs::get<DirectionalLight>(entity);
		glm::vec3 direction = glm::normalize(glm::vec3{V * glm::vec4{light.direction, 0.0f}});

		if (v->toggleCalculateMatrices.value) {
			v->csm.calculateMatrices(v->cameraId, entity);
		}

		v->directionalLightData.appendData(light.ambient, light.color, direction, light.intensity);
		v->directionalLightCount++;
	}
	v->directionalLightData.upload();

	/* TODO: find min-max, create per-tile list of lights */
}

void App::renderShadowsForwardPass()
{
	v->csm.renderCascades();
}

void App::lightingForwardPass()
{
	const auto &projection = ecs::get<Projection>(v->cameraId);
	const auto &P = projection.matrix;
	const auto &view = ecs::get<View>(v->cameraId);
	const auto &V = view.matrix;
	const auto &invV = view.invMatrix;
	const auto VP = P * V;

	const phs::Frustum frustum{VP};

	RN_FB_BIND(v->fbScreenForward);

	v->fbScreenForward.clear(rn::BUFFER_COLOR);

	RN_CHECK(glDepthMask(GL_FALSE));

	v->progLightingForward.use();
	v->progLightingForward.uniform("P", P);
	v->progLightingForward.uniform("V", V);
	v->progLightingForward.uniform("invV", invV);

	v->progLightingForward.uniform("fbScale", glm::vec2{1.f / v->fbScreenForward.width, 1.f / v->fbScreenForward.height});

	size_t unit = 0;
	v->progLightingForward.uniform("texAO", v->ssao.fbAO.color(0)->bind(unit++));
	v->progLightingForward.uniform("lightD", v->directionalLightData.bind(unit++));
	v->progLightingForward.uniform("lightP", v->pointLightData.bind(unit++));
	v->progLightingForward.uniform("lightDCount", v->directionalLightCount);
	v->progLightingForward.uniform("lightPCount", v->pointLightCount);

	GLint csmKernelSizeLocation = v->progLightingForward.getValue("csmKernelSize").id;
	GLint csmBlendCascadesLocation = v->progLightingForward.getValue("csmBlendCascades").id;
	GLint csmSplitsLocation = v->progLightingForward.getValue("csmSplits").id;
	GLint csmCascadesLocation = v->progLightingForward.getValue("csmCascades").id;
	GLint csmRadiuses2Location = v->progLightingForward.getValue("csmRadiuses2").id;
	GLint csmCentersLocation = v->progLightingForward.getValue("csmCenters").id;
	GLint csmMVPLocation = v->progLightingForward.getValue("csmMVP").id;
	GLint csmTexDepthsLocation = v->progLightingForward.getValue("csmTexDepths").id;

	v->progLightingForward.var(csmKernelSizeLocation, static_cast<GLint>(v->csm.kernelSize));
	v->progLightingForward.var(csmBlendCascadesLocation, static_cast<GLuint>(v->csm.blendCascades));
	v->progLightingForward.var(csmSplitsLocation, static_cast<GLint>(v->csm.splits));
	v->progLightingForward.var(csmCascadesLocation, v->csm.cascades.data(), v->csm.cascades.size());
	v->progLightingForward.var(csmRadiuses2Location, v->csm.radiuses2.data(), v->csm.radiuses2.size());
	v->progLightingForward.var(csmCentersLocation, v->csm.centersV.data(), v->csm.centersV.size());
	v->progLightingForward.var(csmMVPLocation, v->csm.VPs.data(), v->csm.VPs.size());
	v->progLightingForward.var(csmTexDepthsLocation, v->csm.texDepths->bind(unit++));

	for (auto &entity : ecs::findWith<Transform, Mesh, Material>())
	{
		if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
		{
			continue;
		}

		if (ecs::has<Shading>(entity) && ecs::get<Shading>(entity).group != Shading::GROUP_SHADED)
		{
			continue;
		}

		proc::MeshRenderer::render(entity, v->progLightingForward);
	}

	v->progLightingForward.forgo();

	RN_CHECK(glDepthMask(GL_TRUE));
}

void App::gBufferPass()
{
	RN_SCOPE_DISABLE(GL_BLEND);
	RN_SCOPE_ENABLE(GL_STENCIL_TEST);

	RN_CHECK(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
	RN_CHECK(glStencilMask(0xF));

	RN_FB_BIND(v->fbGBuffer);
	v->fbGBuffer.clear(rn::BUFFER_COLOR | rn::BUFFER_DEPTH | rn::BUFFER_STENCIL);

	const auto &V = ecs::get<View>(v->cameraId).matrix;
	const auto &P = ecs::get<Projection>(v->cameraId).matrix;

	const phs::Frustum frustum{P * V};

	v->progGBuffer.use();
	v->progGBuffer.var("V", V);

	for (auto &entity : ecs::findWith<Transform, Mesh, Material>())
	{
		if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
		{
			continue;
		}

		GLint ref = ecs::has<Stencil>(entity) ? ecs::get<Stencil>(entity).ref : 0;

		RN_CHECK(glStencilFunc(GL_ALWAYS, ref, 0xF));

		proc::MeshRenderer::render(entity, v->progGBuffer);
	}

	// draw light meshes
	RN_CHECK(glStencilFunc(GL_ALWAYS, Stencil::MASK_FLAT, 0xF));

	v->progGBuffer.var("matShininess", 0.f);

	for (auto &entity : ecs::findWith<Transform, Mesh, PointLight>())
	{
		if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
		{
			continue;
		}

		v->progGBuffer.var("matDiffuse", ecs::get<PointLight>(entity).color);
		proc::MeshRenderer::render(entity, v->progGBuffer);
	}

	for (auto &entity : ecs::findWith<Transform, Mesh, DirectionalLight>())
	{
		if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
		{
			continue;
		}

		v->progGBuffer.var("matDiffuse", ecs::get<DirectionalLight>(entity).color);
		proc::MeshRenderer::render(entity, v->progGBuffer);
	}

	v->progGBuffer.forgo();
}

void App::directionalLightsPass()
{
	const auto &projection = ecs::get<Projection>(v->cameraId);
	const auto &P = projection.matrix;
	const auto &view = ecs::get<View>(v->cameraId);
	const auto &V = view.matrix;
	const auto &invV = view.invMatrix;

	const phs::Frustum frustum{P * V};

	v->progDirectionalLight.var("invV", invV);
	v->progDirectionalLight.var("zNear", projection.zNear);
	v->progDirectionalLight.var("zFar", projection.zFar);

	for (auto &entity : ecs::findWith<DirectionalLight>())
	{
		// glm::mat4 shadowMapMVP = makeShadowMap(entity, frustum);
		if (v->toggleCalculateMatrices.value) {
			v->csm.calculateMatrices(v->cameraId, entity);
		}

		v->csm.renderCascades();

		const auto &light = ecs::get<DirectionalLight>(entity);

		{
			// RN_FB_BIND(v->fbScreen);
			// RN_SCOPE_DISABLE(GL_DEPTH_TEST);

			RN_CHECK(glBlendFunc(GL_ONE, GL_ONE));
			RN_SCOPE_ENABLE(GL_STENCIL_TEST);
			RN_CHECK(glStencilFunc(GL_EQUAL, Stencil::MASK_SHADED, Stencil::MASK_ALL));
			RN_CHECK(glStencilMask(0x0));

			v->progDirectionalLight.use();

			v->progDirectionalLight.var("useColor", v->toggleColor.value);

			v->progDirectionalLight.var("lightAmbient", light.ambient);
			v->progDirectionalLight.var("lightColor", light.color);
			v->progDirectionalLight.var("lightDirection", glm::mat3{V} * light.direction);
			v->progDirectionalLight.var("lightIntensity", light.intensity);
			// v->progDirectionalLight.var("shadowmapMVP", shadowMapMVP);
			// v->progDirectionalLight.var("csmMVP", v->csm.Ps[0] * v->csm.V);
			/*
			std::vector<float> csmRadiuses2{};
			csmRadiuses2.resize(v->csm.radiuses.size());

			for (size_t i = 0; i < csmRadiuses2.size(); i++)
			{
				csmRadiuses2[i] = v->csm.radiuses[i] * v->csm.radiuses[i];
			}

			std::vector<glm::vec3> csmCenters{};
			csmCenters.resize(v->csm.centers.size());

			for (size_t i = 0; i < csmCenters.size(); i++)
			{
				csmCenters[i] = glm::vec3{V  * glm::vec4{v->csm.centers[i], 1.f}};
			}

			std::vector<glm::mat4> csmMVP{};
			csmMVP.resize(v->csm.Ps.size());

			for (size_t i = 0; i < csmMVP.size(); i++)
			{
				csmMVP[i] = v->csm.Ps[i] * v->csm.Vs[i];
			}
			*/
			GLsizei unit = 0;
			v->progDirectionalLight.var("texColor", v->fbGBuffer.color(0)->bind(unit++));
			v->progDirectionalLight.var("texNormal", v->fbGBuffer.color(1)->bind(unit++));
			v->progDirectionalLight.var("texZ", v->fbGBuffer.color(2)->bind(unit++));
			v->progDirectionalLight.var("texDepth", v->fbGBuffer.depth()->bind(unit++));
			v->progDirectionalLight.var("texAO", v->ssao.fbAO.color(0)->bind(unit++));

			// v->progDirectionalLight.var("texShadowMoments", v->fbShadowMap.color(0)->bind(unit++));
			// v->progDirectionalLight.var("texShadowDepth", v->fbShadowMap.depth()->bind(unit++));
			// v->progDirectionalLight.var("texCSM", v->csm.fbShadows[0].color(0)->bind(unit++));

			v->progDirectionalLight.var("csmSplits", static_cast<GLint>(v->csm.splits));
			v->progDirectionalLight.var("csmCascades", v->csm.cascades.data(), v->csm.cascades.size());
			// v->progDirectionalLight.var("csmRadiuses2", v->csmRadiuses2.data(), v->csmRadiuses2.size());
			v->progDirectionalLight.var("csmRadiuses2", v->csm.radiuses2.data(), v->csm.radiuses2.size());
			// v->progDirectionalLight.var("csmCenters", v->csmCenters.data(), v->csmCenters.size());
			v->progDirectionalLight.var("csmCenters", v->csm.centersV.data(), v->csm.centersV.size());
			// v->progDirectionalLight.var("csmMVP", v->csmMVP.data(), v->csmMVP.size());
			v->progDirectionalLight.var("csmMVP", v->csm.VPs.data(), v->csm.VPs.size());
			// v->progDirectionalLight.var("csmTexCascades", v->csm.texCascades->bind(unit++));
			v->progDirectionalLight.var("csmTexDepths", v->csm.texDepths->bind(unit++));

			rn::Mesh::quad.render();

			v->progDirectionalLight.forgo();

			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		}
	}
}

void App::pointLightsPass()
{
	glm::mat4 &V = ecs::get<View>(v->cameraId).matrix;

	for (auto &entity : ecs::findWith<Transform, Mesh, PointLight>())
	{
		const auto &light = ecs::get<PointLight>(entity);
		const auto &lightTransform = ecs::get<Transform>(entity);

		const auto lightPosition = V * glm::vec4{lightTransform.translation, 1.f};

		// @TODO: shadows
		// {
		// 	RN_FB_BIND(shadowmap);

		// 	shadowmap.clear();

		// 	v->progShadowMap.use();
		// 	v->progShadowMap.var("V", V);

		// 	for (auto &entity : ecs::findWith<Transform, Mesh, Occluder>())
		// 	{
		// 		proc::MeshRenderer::render(entity, v->progShadowMap);
		// 	}

		// 	v->progShadowMap.forgo();
		// }

		{
			// RN_FB_BIND(v->fbScreen);

			RN_SCOPE_ENABLE(GL_STENCIL_TEST);
			RN_CHECK(glStencilFunc(GL_EQUAL, Stencil::MASK_SHADED, Stencil::MASK_ALL));
			RN_CHECK(glStencilMask(0x0));

			RN_CHECK(glBlendFunc(GL_ONE, GL_ONE));

			v->progPointLight.use();
			v->progPointLight.var("lightPosition", lightPosition);
			v->progPointLight.var("lightColor", light.color);
			v->progPointLight.var("lightIntensity", light.intensity);
			v->progPointLight.var("lightLinearAttenuation", light.linearAttenuation);
			v->progPointLight.var("lightQuadraticAttenuation", light.quadraticAttenuation);

			v->progPointLight.var("texColor", v->fbGBuffer.color(0)->bind(0));
			v->progPointLight.var("texNormal", v->fbGBuffer.color(1)->bind(1));
			v->progPointLight.var("texZ", v->fbGBuffer.color(2)->bind(2));
			v->progPointLight.var("texDepth", v->fbGBuffer.depth()->bind(3));

			// shadowMapBuffer.colors[0].bind(3);
			// shadowMapBuffer.depth.tex.bind(4);

			// v->progPointLight.var("shadowMoments", 3);
			// v->progPointLight.var("shadowDepth", 4);

			rn::Mesh::quad.render();

			v->progPointLight.forgo();

			RN_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		}
	}
}

void App::flatLightPass()
{
	RN_SCOPE_ENABLE(GL_STENCIL_TEST);
	RN_CHECK(glStencilFunc(GL_EQUAL, Stencil::MASK_FLAT, Stencil::MASK_ALL));
	RN_CHECK(glStencilMask(0x0));

	v->progFlatLight.use();

	v->progFlatLight.var("texColor", v->fbGBuffer.color(0)->bind(0));

	rn::Mesh::quad.render();

	v->progFlatLight.forgo();
}

void App::ssaoPass()
{
	v->ssao.clear();

	if ( ! v->toggleSSAO.value)
	{
		return;
	}

	rn::Tex *texDepth;
	rn::Tex *texNormal;

	if ( ! v->toggleDeferred.value)
	{
		texDepth = v->fbZPrefill.depth();
		texNormal = v->fbZPrefill.color(0);
	}
	else
	{
		texDepth = v->fbGBuffer.depth();
		texNormal = v->fbGBuffer.color(1);
	}

	v->ssao.genMipMaps(texDepth);
	v->ssao.computeAO(texNormal);

	if (v->toggleSSAOBlur.value) {
		v->ssao.blur();
	}
}
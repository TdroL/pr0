#include "app.hpp"

#include <core/ecs/entity.hpp>

#include <core/cull/raster.hpp>

#include <core/asset/mesh.hpp>
#include <core/event.hpp>
#include <core/ngn.hpp>
#include <core/ngn/fs.hpp>
#include <core/ngn/key.hpp>
#include <core/ngn/window.hpp>
#include <core/phs/frustum.hpp>
#include <core/rn.hpp>
#include <core/rn/fb.hpp>
#include <core/rn/font.hpp>
#include <core/rn/format.hpp>
#include <core/rn/mesh.hpp>
#include <core/rn/prof.hpp>
#include <core/rn/program.hpp>
#include <core/rn/ssb.hpp>
#include <core/src/mem.hpp>
#include <core/src/sbm.hpp>
#include <core/util/count.hpp>
#include <core/util/timer.hpp>
#include <core/util/toggle.hpp>

#include <app/fx/ssao.hpp>
#include <app/fx/csm.hpp>
#include <app/scene.hpp>
#include <app/events.hpp>

#include <app/comp/boundingvolume.hpp>
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
#include <app/comp/temporaltransform.hpp>
#include <app/comp/transform.hpp>
#include <app/comp/view.hpp>

#include <app/proc/camera.hpp>
#include <app/proc/frustumprocess.hpp>
#include <app/proc/inputprocess.hpp>
#include <app/proc/meshrenderer.hpp>
#include <app/proc/rebuildboundingvolumeprocess.hpp>
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

class App::Variables
{
public:
	cull::Raster raster;

	ecs::Entity cameraId{};
	ecs::Entity lightIds[10]{};

	rn::Font font1{"DejaVuSansMono"};
	rn::Font font2{"DejaVuSansMono"};

	rn::Program progZPrefill{"App::progZPrefill"};
	rn::Program progZDebug{"App::progZDebug"};
	rn::Program progLightingForward{"App::progLightingForward"};
	rn::Program progFBBlit{"App::progFBBlit"};

	rn::FB fbZPrefill{"App::fbZPrefill"};
	rn::FB fbScreenForward{"App::fbScreenForward"};

	rn::Prof profZPrefill{"App::profZPrefill"};
	rn::Prof profSetupLights{"App::profSetupLights"};
	rn::Prof profRenderShadows{"App::profRenderShadows"};
	rn::Prof profLighting{"App::profLighting"};

	rn::SSB ssbPointLight{"App::ssbPointLight"};
	rn::SSB ssbDirectionalLight{"App::ssbDirectionalLight"};

	rn::FB fbUI{"App::fbUI"};
	// rn::FB fbShadowMap{"App::fbShadowMap"};
	// rn::FB fbShadowMapBlur{"App::fbShadowMapBlur"};

	fx::SSAO ssao{};
	fx::CSM csm{};

	rn::Prof profRender{"App::profRender"};
	rn::Prof profSSAO{"App::profSSAO"};
	rn::Prof profCSM{"App::profCSM"};

	app::Scene scene{};

	util::Timer sunTimer{};

	util::Toggle toggleSSAO{"SSAO (b)", 0};
	util::Toggle toggleSSAOBlur{"SSAOBlur (n)", 1};
	util::Toggle toggleDebugPreview{"DebugPreview (m)", 0};
	util::Toggle toggleZPreview{"ZPreview (,)", 0};
	util::Toggle toggleLights{"Lights (;)", 1};
	util::Toggle toggleColor{"Color (/)", 0};
	util::Toggle toggleCascade{"Cascade (')", 0, 4};
	util::Toggle toggleCalculateMatrices{"CalculateMatrices ([)", 1};

	event::Listener<win::WindowResizeEvent> listenerWindowResize{};
	event::Listener<ProjectionChangedEvent> listenerProjectionChanged{};

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
	: v{new Variables{}}
{}

App::~App() = default;

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

	v->profZPrefill.init();
	v->profSetupLights.init();
	v->profRenderShadows.init();
	v->profLighting.init();

	v->profRender.init();
	v->profSSAO.init();
	v->profCSM.init();

	/* Test: switch to window mode */
	UTIL_DEBUG
	{
		v->currentMode = 0;
		v->currentVsync = 1;

		clog << "Test: switching to " << v->modeNames[v->currentMode] << " " << v->vsyncNames[v->currentVsync] << endl;

		win::switchMode(v->modes[v->currentMode], v->vsyncs[v->currentVsync]);
		rn::reloadSoftAll();
	}

	glm::vec3 polygon[3] {
		{0.25f, 0.75f, 1.f},
		{0.5f, 0.125f, 0.5f},
		{-0.5f, -0.75f, 0.25f}
	};

	v->raster.reset({16, 9}); // 256, 144
	v->raster.clear();
	v->raster.draw(polygon);
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
}

void App::initFB()
{
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

	v->listenerWindowResize.attach([&] (const win::WindowResizeEvent &)
	{
		// fbZPrefill
		{
			auto texNormals = dynamic_cast<rn::Tex2D *>(v->fbZPrefill.color(0));
			texNormals->width = win::internalWidth;
			texNormals->height = win::internalHeight;
			texNormals->reload();
		}

		{
			auto texNormals = dynamic_cast<rn::Tex2D *>(v->fbZPrefill.color(1));
			texNormals->width = win::internalWidth;
			texNormals->height = win::internalHeight;
			texNormals->reload();
		}

		{
			auto texDepth = dynamic_cast<rn::Tex2D *>(v->fbZPrefill.depth());
			texDepth->width = win::internalWidth;
			texDepth->height = win::internalHeight;
			texDepth->reload();
		}

		v->fbZPrefill.width = win::internalWidth;
		v->fbZPrefill.height = win::internalHeight;
		v->fbZPrefill.reload();

		// fbScreenForward
		{
			auto texColor = dynamic_cast<rn::Tex2D *>(v->fbScreenForward.color(0));
			texColor->width = win::internalWidth;
			texColor->height = win::internalHeight;
			texColor->reload();
		}

		v->fbScreenForward.width = win::internalWidth;
		v->fbScreenForward.height = win::internalHeight;
		v->fbScreenForward.reload();

		// fbUI
		{
			auto texColor = dynamic_cast<rn::Tex2D *>(v->fbUI.color(0));
			texColor->width = win::internalWidth;
			texColor->height = win::internalHeight;
			texColor->reload();
		}

		v->fbUI.width = win::internalWidth;
		v->fbUI.height = win::internalHeight;
		v->fbUI.reload();
	});

	v->ssbPointLight.reload();
	v->ssbDirectionalLight.reload();

	v->ssbPointLight.bind(10);
	v->ssbDirectionalLight.bind(11);
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
		projection.aspect = static_cast<float>(win::internalWidth) / static_cast<float>(win::internalHeight);
		projection.matrix = glm::infiniteReversePerspective(glm::radians(projection.fovy), projection.aspect, projection.zNear);
		projection.invMatrix = glm::inverseInfiniteReversePerspective(glm::radians(projection.fovy), projection.aspect, projection.zNear);

		v->listenerWindowResize.attach([&] (const win::WindowResizeEvent &)
		{
			projection.aspect = static_cast<float>(win::internalWidth) / static_cast<float>(win::internalHeight);
			projection.matrix = glm::infiniteReversePerspective(glm::radians(projection.fovy), projection.aspect, projection.zNear);
			projection.invMatrix = glm::inverseInfiniteReversePerspective(glm::radians(projection.fovy), projection.aspect, projection.zNear);

			event::emit(ProjectionChangedEvent{projection, win::internalWidth, win::internalHeight});
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
		light.radius = 1.0;
		light.cutoff = 3.0;
		// linear attenuation; distance at which half of the light intensity is lost
		// light.linearAttenuation = 1.0 / pow(3.0, 2); // r_l = 3.0;
		// quadriatic attenuation; distance at which three-quarters of the light intensity is lost
		// light.quadraticAttenuation = 1.0 / pow(6.0, 2); // r_q = 6.0;

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
		light.radius = 1.5;
		light.cutoff = light.radius * 3.0;
		// linear attenuation; distance at which half of the light intensity is lost
		// light.linearAttenuation = 1.0 / pow(0.5, 2); // r_l = 3.0;
		// quadriatic attenuation; distance at which three-quarters of the light intensity is lost
		// light.quadraticAttenuation = 1.0 / pow(1.0, 2); // r_q = 6.0;

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
		light.radius = 1.5;
		light.cutoff = 2.0;
		// linear attenuation; distance at which half of the light intensity is lost
		// light.linearAttenuation = 1.0 / pow(1.5, 2); // r_l = 3.0;
		// quadriatic attenuation; distance at which three-quarters of the light intensity is lost
		// light.quadraticAttenuation = 1.0 / pow(4.0, 2); // r_q = 6.0;

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
		light.radius = 1.5; // r_l = 3.0;
		light.cutoff = light.radius * 3.0;

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
		light.ambient = glm::vec4{0.f, 0.0036f, 0.005f, 1.f};
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
		proc::RebuildBoundingVolumeProcess::update(entity);
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
		rn::FB::resetAll();
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

		rn::resetAllContextRelated();

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
		lightPositionDelta.z *= (ngn::key::pressed(KEY_KP_8)   - ngn::key::pressed(KEY_KP_5));

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
		proc::RebuildBoundingVolumeProcess::update(entity);
	}
}

void App::render()
{
	RN_CHECK(glClearColor(0.f, 0.f, 0.f, 1.f));
	RN_CHECK(glClearDepth(0.0));
	RN_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

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
		oss << "    CSM=" << v->profCSM.ms() << "ms\n";
		// oss << "      Render=" << v->csm.profRender.ms() << "ms\n";
		// oss << "      Blur=" << v->csm.profBlur.ms() << "ms\n";
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

void App::zPrefillForwardPass()
{
	const auto &projection = ecs::get<Projection>(v->cameraId);
	const auto &P = projection.matrix;
	const auto &view = ecs::get<View>(v->cameraId);
	const auto &V = view.matrix;

	const phs::Frustum frustum{P, V};

	// RN_SCOPE_DISABLE(GL_BLEND);

	RN_FB_BIND(v->fbZPrefill);

	// v->fbZPrefill.bind();
	v->fbZPrefill.clear(rn::BUFFER_COLOR | rn::BUFFER_DEPTH | rn::BUFFER_STENCIL);

	v->progZPrefill.use();
	v->progZPrefill.uniform("P", P);
	v->progZPrefill.uniform("V", V);

	GLint locationM = v->progZPrefill.getUniformMeta("M").location;

	for (auto &entity : ecs::findWith<Transform, Mesh, Material>())
	{
		if (ecs::has<BoundingVolume>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
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
	const auto &projection = ecs::get<Projection>(v->cameraId);
	const auto &V = view.matrix;

	struct PointLightData
	{
		glm::vec4 color{0.f};
		glm::vec4 position{0.f};
		float intensity = 0.f;
		float radius = 0.f;
		float cutoff = 0.f;
		float padding[1]{ 0.f };
	};
	static_assert(sizeof(PointLightData) % sizeof(glm::vec4) == 0, "Size of PointLightData must be padded to size of vec4");

	v->ssbPointLight.clear();
	for (auto &entity : ecs::findWith<PointLight, Transform>())
	{
		const auto &light = ecs::get<PointLight>(entity);
		glm::vec4 position{V * glm::vec4{ecs::get<Transform>(entity).translation, 1.0f}};

		v->ssbPointLight.appendData(PointLightData{
			glm::pow(light.color, glm::vec4{2.2f}),
			position,
			light.intensity,
			light.radius,
			light.cutoff,
			0.f
		});
	}
	v->ssbPointLight.upload();

	struct DirectionalLightData
	{
		glm::vec4 ambient{0.f};
		glm::vec4 color{0.f};
		glm::vec3 direction{0.f};
		float intensity = 0.f;
	};
	static_assert(sizeof(DirectionalLightData) % sizeof(glm::vec4) == 0, "Size of DirectionalLightData must be padded to size of vec4");

	v->ssbDirectionalLight.clear();
	for (auto &entity : ecs::findWith<DirectionalLight>())
	{
		const auto &light = ecs::get<DirectionalLight>(entity);
		glm::vec3 direction = glm::normalize(glm::vec3{V * glm::vec4{light.direction, 0.0f}});

		float zMax = -numeric_limits<float>::max();
		for (auto &entity : ecs::findWith<Transform, Mesh, BoundingVolume, Occluder>())
		{
			auto &boundingVolume = ecs::get<BoundingVolume>(entity);
			float dist = glm::dot(direction, boundingVolume.sphere.position) + boundingVolume.sphere.radius;

			zMax = max(zMax, dist);
		}

		if (v->toggleCalculateMatrices.value) {
			v->csm.calculateMatrices(light, projection, view, zMax);
		}

		v->ssbDirectionalLight.appendData(DirectionalLightData{
			glm::pow(light.ambient, glm::vec4{2.2f}),
			glm::pow(light.color, glm::vec4{2.2f}),
			direction,
			light.intensity
		});
	}
	v->ssbDirectionalLight.upload();

	/* TODO: create per-tile list of lights */
}

void App::renderShadowsForwardPass()
{
	v->profCSM.start();

	// v->csm.renderCascades();

	v->csm.progDepth.use();

	RN_CHECK(glDepthFunc(GL_LEQUAL));
	RN_CHECK(glCullFace(GL_FRONT));

	for (size_t i = 0; i < v->csm.splits; i++)
	{
		const auto &P = v->csm.Ps[i];
		const auto &V = v->csm.Vs[i];
		const auto VP = P * V;

		v->csm.progDepth.uniform("P", P);
		v->csm.progDepth.uniform("V", V);

		auto &fbShadow = v->csm.fbShadows[i];

		RN_FB_BIND(fbShadow);
		fbShadow.clear(rn::BUFFER_DEPTH);

		const phs::Frustum boundingFrustum{VP};

		for (auto &entity : ecs::findWith<Transform, Mesh, Occluder>())
		{
			if (ecs::has<BoundingVolume>(entity) && ! proc::FrustumProcess::isVisible(entity, boundingFrustum))
			{
				continue;
			}

			proc::MeshRenderer::render(entity, v->csm.progDepth);
		}
	}

	RN_CHECK(glCullFace(rn::Default::cullFace));
	RN_CHECK(glDepthFunc(rn::Default::depthFunc));

	v->csm.progDepth.forgo();

	v->profCSM.stop();
}

void App::lightingForwardPass()
{
	const auto &projection = ecs::get<Projection>(v->cameraId);
	const auto &view = ecs::get<View>(v->cameraId);
	const auto &P = projection.matrix;
	const auto &V = view.matrix;
	const auto &invV = view.invMatrix;

	const phs::Frustum frustum{P, V};

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

	GLint csmBlendCascadesLocation = v->progLightingForward.getUniformMeta("csm.blendCascades").location;
	GLint csmCentersLocation = v->progLightingForward.getUniformMeta("csm.centers").location;
	GLint csmKernelSizeLocation = v->progLightingForward.getUniformMeta("csm.kernelSize").location;
	GLint csmMVPLocation = v->progLightingForward.getUniformMeta("csm.MVP").location;
	GLint csmRadiuses2Location = v->progLightingForward.getUniformMeta("csm.radiuses2").location;
	GLint csmSplitsLocation = v->progLightingForward.getUniformMeta("csm.splits").location;
	GLint csmTexDepthsLocation = v->progLightingForward.getUniformMeta("csmTexDepths").location;

	v->progLightingForward.var(csmBlendCascadesLocation, static_cast<GLuint>(v->csm.blendCascades));
	v->progLightingForward.var(csmCentersLocation, v->csm.centers.data(), v->csm.centers.size());
	v->progLightingForward.var(csmKernelSizeLocation, static_cast<GLuint>(v->csm.kernelSize));
	v->progLightingForward.var(csmMVPLocation, v->csm.shadowBiasedVPs.data(), v->csm.shadowBiasedVPs.size());
	v->progLightingForward.var(csmRadiuses2Location, v->csm.radiuses2.data(), v->csm.radiuses2.size());
	v->progLightingForward.var(csmSplitsLocation, static_cast<GLuint>(v->csm.splits));
	v->progLightingForward.var(csmTexDepthsLocation, v->csm.texDepths->bind(unit++));

	for (auto &entity : ecs::findWith<Transform, Mesh, Material>())
	{
		if (ecs::has<BoundingVolume>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
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

void App::ssaoPass()
{
	v->ssao.clear();

	if ( ! v->toggleSSAO.value)
	{
		return;
	}

	rn::Tex *texDepth = v->fbZPrefill.depth();
	rn::Tex *texNormal = v->fbZPrefill.color(0);

	if (texDepth == nullptr)
	{
		UTIL_DEBUG
		{
			clog << "App::ssaoPass(): missing texDepth texture" << endl;
		}

		return;
	}

	if (texNormal == nullptr)
	{
		UTIL_DEBUG
		{
			clog << "App::ssaoPass(): missing texNormal texture" << endl;
		}

		return;
	}

	v->ssao.genMipMaps(*texDepth);
	v->ssao.computeAO(*texNormal);

	if (v->toggleSSAOBlur.value) {
		v->ssao.blur();
	}
}
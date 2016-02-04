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
#include <core/rn/math.hpp>
#include <core/rn/mesh.hpp>
#include <core/rn/prof.hpp>
#include <core/rn/program.hpp>
#include <core/rn/ssb.hpp>
#include <core/rn/ub.hpp>
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
	cull::Raster raster{};

	ecs::Entity cameraId{};
	ecs::Entity lightIds[10]{};
	ecs::Entity bvSphereId{};

	rn::Font font1{"DejaVuSansMono"};
	rn::Font font2{"DejaVuSansMono"};

	rn::Program progZPrefill{"App::progZPrefill"};
	rn::Program progZDebug{"App::progZDebug"};
	rn::Program progLightingForward{"App::progLightingForward"};
	rn::Program progGammaCorrection{"App::progGammaCorrection"};
	rn::Program progFBBlit{"App::progFBBlit"};
	rn::Program progWireframe{"App::progWireframe"};

	rn::FB fbZPrefill{"App::fbZPrefill"};
	rn::FB fbScreen{"App::fbScreen"};

	rn::Prof profZPrefill{"App::profZPrefill"};
	rn::Prof profSetupLights{"App::profSetupLights"};
	rn::Prof profRenderShadows{"App::profRenderShadows"};
	rn::Prof profLighting{"App::profLighting"};
	rn::Prof profPostProcessing{"App::profPostProcessing"};
	rn::Prof profRender{"App::profRender"};
	rn::Prof profSSAO{"App::profSSAO"};
	rn::Prof profCSM{"App::profCSM"};
	rn::Prof profCSMLayers[fx::CSM::maxCascades];
	rn::Prof profWireframe{"App::profWireframe"};

	rn::SSB ssbPointLight{"App::ssbPointLight"};
	rn::SSB ssbDirectionalLight{"App::ssbDirectionalLight"};

	rn::UB ubMatrices{"App::ubMatrices"};
	rn::UB ubCSM{"App::ubCSM"};

	rn::FB fbUI{"App::fbUI"};

	fx::SSAO ssao{};
	fx::CSM csm{};

	app::Scene scene{};

	util::Timer sunTimer{};

	util::Toggle toggleSSAO{"SSAO (b)", 0};
	util::Toggle toggleWireframe{"BV Wireframe (n)", 0};
	util::Toggle toggleShadowFaceCulling{"Shadow face culling (m)", 0};

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

	const int vSyncs[3]
	{
		-1, // progressive
		 0, // off
		 1, // on
	};

	const string vSyncNames[3]
	{
		"progressive",
		"off",
		"on",
	};

	size_t currentVSync = 0;
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
	v->profPostProcessing.init();
	v->profRender.init();
	v->profSSAO.init();
	v->profCSM.init();
	for (size_t i = 0; i < fx::CSM::maxCascades; i++)
	{
		v->profCSMLayers[i].profName = "App::profCSMLayers[" + to_string(i) + "]";
		v->profCSMLayers[i].init();
	}

	/* Test: switch to window mode */
	UTIL_DEBUG
	{
		v->currentMode = 0;
		v->currentVSync = 1;

		clog << "Test: switching to " << v->modeNames[v->currentMode] << " " << v->vSyncNames[v->currentVSync] << endl;

		win::switchMode(v->modes[v->currentMode], v->vSyncs[v->currentVSync]);
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

	try
	{
		v->progGammaCorrection.load("lighting/forward/gammaCorrection.frag", "lighting/forward/gammaCorrection.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		v->progWireframe.load("rn/wireframe.frag", "rn/wireframe.vert");
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
		auto texDepth = static_pointer_cast<rn::Tex2D>(v->fbZPrefill.shareDepth());
		v->fbScreen.attachDepth(texDepth);
	}

	v->fbScreen.clearColorValue = glm::vec4{0.f, 0.f, 0.f, 0.f};
	v->fbScreen.reload();

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

		// fbScreen
		{
			auto texColor = dynamic_cast<rn::Tex2D *>(v->fbScreen.color(0));
			texColor->width = win::internalWidth;
			texColor->height = win::internalHeight;
			texColor->reload();
		}

		v->fbScreen.width = win::internalWidth;
		v->fbScreen.height = win::internalHeight;
		v->fbScreen.reload();

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

	v->ssbPointLight.bind(0);
	v->ssbDirectionalLight.bind(1);

	v->ubMatrices.reload();
	v->ubCSM.reload();

	v->ubMatrices.bind(0);
	v->ubCSM.bind(1);
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
		rn::math::infRevPerspectiveMatrixAndInverse(projection.matrix, projection.invMatrix, glm::radians(projection.fovy), projection.aspect, projection.zNear);

		v->listenerWindowResize.attach([&] (const win::WindowResizeEvent &)
		{
			projection.aspect = static_cast<float>(win::internalWidth) / static_cast<float>(win::internalHeight);
			rn::math::infRevPerspectiveMatrixAndInverse(projection.matrix, projection.invMatrix, glm::radians(projection.fovy), projection.aspect, projection.zNear);

			event::emit(ProjectionChangedEvent{projection, win::internalWidth, win::internalHeight});
		});

		// create camera view matrices
		proc::Camera::update(v->cameraId, glm::vec3{0.f}, glm::vec3{0.f});
	}

	// create lights
	v->lightIds[0] = ecs::create();
	v->lightIds[1] = ecs::create();
	v->lightIds[2] = ecs::create();
	v->lightIds[3] = ecs::create();

	// directional lights
	v->lightIds[4] = ecs::create();
	v->lightIds[5] = ecs::create();

	// light #1
	{
		ecs::enable<Name, PointLight, Position, Transform, Mesh>(v->lightIds[0]);

		auto &name = ecs::get<Name>(v->lightIds[0]);
		name.name = "PointLight #1";

		auto &light = ecs::get<PointLight>(v->lightIds[0]);
		light.color = glm::vec4{1.f, 1.f, 1.f, 1.f};
		light.radius = 1.0;
		light.cutoff = 3.0;

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
		light.color = glm::vec4{0.8f, 0.8f, 0.8f, 1.f};
		light.ambient = glm::vec4{light.color.rgb() / 127.f, 1.f};
		light.direction = glm::vec3{0.5f, 0.5f, 1.f};
		light.intensity = 4.f;
		light.diffuseOnly = false;
		light.shadowCaster = true;

		auto &transform = ecs::get<Transform>(v->lightIds[4]);
		transform.translation = light.direction * 100.f;
		transform.scale = glm::vec3{10.f};

		ecs::get<Mesh>(v->lightIds[4]).id = asset::mesh::load("sphere.sbm");
	}

	// directional light #2 (sky fill)
	if (false)
	{
		ecs::enable<Name, DirectionalLight>(v->lightIds[5]);

		auto &name = ecs::get<Name>(v->lightIds[5]);
		name.name = "SkyFill";

		auto &light = ecs::get<DirectionalLight>(v->lightIds[5]);
		light.color = glm::vec4{110.f / 255.f, 205.f / 255.f, 253.f / 255.f, 1.f};
		light.ambient = glm::vec4{light.color.rgb() / 4.f, 1.f};
		light.direction = glm::vec3{0.0f, 1.0f, 0.f};
		light.intensity = 0.0625f;
		light.diffuseOnly = true;
		light.shadowCaster = false;
	}

	// bounding volume meshes
	{
		v->bvSphereId = ecs::create();
		ecs::enable<Name, Mesh>(v->bvSphereId);
		ecs::get<Name>(v->bvSphereId).name = "Bounding Volume Sphere";
		ecs::get<Mesh>(v->bvSphereId).id = asset::mesh::load("bv-sphere.sbm");
	}

	// build bounding volumes for renderable entities
	for (auto &entity : ecs::findWith<Name, Transform, Mesh>())
	{
		proc::RebuildBoundingVolumeProcess::update(entity);
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
		v->currentVSync = (v->currentVSync + 1) % util::countOf(v->vSyncs);

		cout << "Switching vSync mode to \"" << v->vSyncNames[v->currentVSync] << "\" (" << v->vSyncs[v->currentVSync] << ") ..." << endl;

		rn::resetAllContextRelated();

		win::switchMode(v->modes[v->currentMode], v->vSyncs[v->currentVSync]);

		rn::reloadSoftAll();

		cout << "done" << endl;
	}

	if (key::hit(KEY_F11))
	{
		cout << "Switching window mode..." << endl;
		v->currentMode = (v->currentMode + 1) % util::countOf(v->modes);
		win::switchMode(v->modes[v->currentMode], v->vSyncs[v->currentVSync]);
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
		v->toggleWireframe.change();
	}

	if (key::hit('m'))
	{
		v->toggleShadowFaceCulling.change();
	}

	if (key::hit('7'))
	{
		v->csm.lambda -= 0.01f;
	}

	if (key::hit('8'))
	{
		v->csm.lambda -= 0.001f;
	}

	if (key::hit('9'))
	{
		v->csm.lambda += 0.001f;
	}

	if (key::hit('0'))
	{
		v->csm.lambda += 0.01f;
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

	v->sunTimer.update(key::pressed(KEY_LSHIFT) ? 0.05f : 1.f);

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
	zPrefillPass();
	v->profZPrefill.stop();

	v->profSSAO.start();
	ssaoPass();
	v->profSSAO.stop();

	v->profSetupLights.start();
	setupLightsPass();
	v->profSetupLights.stop();

	v->profRenderShadows.start();
	renderShadowMapsPass();
	v->profRenderShadows.stop();

	v->profLighting.start();
	forwardLightingPass();
	v->profLighting.stop();

	v->profWireframe.start();
	wireframePass();
	v->profWireframe.stop();

	v->profPostProcessing.start();
	postProcessingPass();
	v->profPostProcessing.stop();

	if (false)
	{
		RN_SCOPE_DISABLE(GL_DEPTH_TEST);

		v->progZDebug.use();
		v->progZDebug.uniform("texNormal", v->fbZPrefill.color(0)->bind(0));
		v->progZDebug.uniform("texDebug", v->fbZPrefill.color(1)->bind(1));
		v->progZDebug.uniform("texScreen", v->fbScreen.color(0)->bind(2));

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
		oss << "    CSM=" << v->profCSM.ms() << "ms/" << v->profCSM.latency() << "f\n";
		for (size_t i = 0; i < v->csm.splits; i++)
		{
			oss << "      #" << i << "=" << v->profCSMLayers[i].ms() << "ms/" << v->profCSMLayers[i].latency() << "f\n";
		}
		// oss << "      Render=" << v->csm.profRender.ms() << "ms\n";
		// oss << "      Blur=" << v->csm.profBlur.ms() << "ms\n";
		oss << "  Lighting=" << v->profLighting.ms() << "ms/" << v->profLighting.latency() << "f\n";
		oss << "  Wireframe=" << v->profWireframe.ms() << "ms/" << v->profWireframe.latency() << "f\n";
		oss << "  Post=" << v->profPostProcessing.ms() << "ms/" << v->profPostProcessing.latency() << "f\n";
		oss << "\n";
		oss << "F4 - reload scene\n";
		oss << "F5 - reload shaders\n";
		oss << "F6 - reload meshes\n";
		oss << "F7 - reload fonts\n";
		oss << "F8 - reload FBOs\n";
		oss << "F9 - reload textures\n";
		oss << "F10 - change VSync mode (current: " << v->vSyncNames[v->currentVSync] << ")\n";
		oss << "F11 - change window mode (current: " << v->modeNames[v->currentMode] << ")\n";
		oss << "\n";
		oss << "Movement: W, A, S, D, SPACE, CTRL (SHIFT - slower movement)\n";
		oss << "Camera: arrows (SHIFT - slower rotation)\n";
		oss << "Point lights: Keypad 8, 4, 5, 6; i, j, k, l\n";
		oss << "PSSM lambda: 7, 8, 9, 0 (current: " << v->csm.lambda << ")\n\n";
		oss << "Toggles:\n";

		for (auto &toggle : util::Toggle::collection)
		{
			oss << "  " << toggle->toggleName << " = " << toggle->value << "\n";
		}

		v->font1.render(oss.str());

		oss.str(""s);
		oss.clear();

		oss << "CSM:\n";
		oss << "  V[0][0]=" << glm::to_string(glm::row(v->csm.Vs[0], 0)) << "\n";
		oss << "  V[0][1]=" << glm::to_string(glm::row(v->csm.Vs[0], 1)) << "\n";
		oss << "  V[0][2]=" << glm::to_string(glm::row(v->csm.Vs[0], 2)) << "\n";
		oss << "  V[0][3]=" << glm::to_string(glm::row(v->csm.Vs[0], 3)) << "\n";
		oss << "\n";
		oss << "  P[0][0]=" << glm::to_string(glm::row(v->csm.Ps[0], 0)) << "\n";
		oss << "  P[0][1]=" << glm::to_string(glm::row(v->csm.Ps[0], 1)) << "\n";
		oss << "  P[0][2]=" << glm::to_string(glm::row(v->csm.Ps[0], 2)) << "\n";
		oss << "  P[0][3]=" << glm::to_string(glm::row(v->csm.Ps[0], 3)) << "\n";

		v->font2.position.x = -0.125f;
		v->font2.render(oss.str());

		v->fbUI.unbind();

		RN_SCOPE_ENABLE(GL_BLEND);
		RN_SCOPE_DISABLE(GL_DEPTH_TEST);
		RN_CHECK(glBlendFuncSeparate(rn::Default::blendFuncSrcRGB, rn::Default::blendFuncDstRGB, rn::Default::blendFuncSrcA, rn::Default::blendFuncDstA));

		v->progFBBlit.use();
		v->progFBBlit.uniform("texSource", v->fbUI.color(0)->bind(0));
		rn::Mesh::quad.render();
		v->progFBBlit.forgo();

		// RN_CHECK(glBlendFuncSeparate(rn::Default::blendFuncSrcRGB, rn::Default::blendFuncDstRGB, rn::Default::blendFuncSrcA, rn::Default::blendFuncDstA));
	}
}

void App::zPrefillPass()
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
	auto locationM = v->progZPrefill.getUniformLocation("M");

	for (auto &entity : ecs::findWith<Transform, Mesh, Material>())
	{
		if (ecs::has<BoundingVolume>(entity) && ! proc::FrustumProcess::isVisible(entity, frustum))
		{
			continue;
		}

		auto &transform = ecs::get<Transform>(entity);

		glm::mat4 M{1.f};
		M = glm::translate(M, transform.translation);
		M = glm::rotate(M, glm::radians(transform.rotation.x), glm::vec3{1.f, 0.f, 0.f});
		M = glm::rotate(M, glm::radians(transform.rotation.y), glm::vec3{0.f, 1.f, 0.f});
		M = glm::rotate(M, glm::radians(transform.rotation.z), glm::vec3{0.f, 0.f, 1.f});
		M = glm::scale(M, transform.scale);

		v->progZPrefill.var(locationM, M);

		proc::MeshRenderer::render(entity);
	}

	v->progZPrefill.forgo();

	// v->fbZPrefill.unbind();
}

void App::setupLightsPass()
{
	const auto &view = ecs::get<View>(v->cameraId);
	const auto &projection = ecs::get<Projection>(v->cameraId);
	const auto &V = view.matrix;

	// Point lights

	struct PointLightData
	{
		glm::vec3 color;
		GLfloat intensity;
		glm::vec3 position;
		GLfloat radius;
		GLfloat cutoff;
		GLfloat padding[3];
	};
	static_assert(sizeof(PointLightData) % sizeof(glm::vec4) == 0, "Size of PointLightData must be padded to size of vec4");

	v->ssbPointLight.clear();
	for (auto &entity : ecs::findWith<PointLight, Transform>())
	{
		const auto &light = ecs::get<PointLight>(entity);
		glm::vec4 position{V * glm::vec4{ecs::get<Transform>(entity).translation, 1.0f}};

		PointLightData *data = v->ssbPointLight.mapData<PointLightData>();
		data->color = glm::pow(light.color.rgb(), glm::vec3{2.2f});
		data->position = position.xyz();
		data->intensity = light.intensity;
		data->radius = light.radius;
		data->cutoff = light.cutoff;
	}
	v->ssbPointLight.upload();

	// Directional lights

	struct DirectionalLightData
	{
		glm::vec3 ambient;
		GLfloat intensity;
		glm::vec3 color;
		GLuint diffuseOnly;
		glm::vec3 direction;
		GLuint shadowCaster;
	};
	static_assert(sizeof(DirectionalLightData) % sizeof(glm::vec4) == 0, "Size of DirectionalLightData must be padded to size of vec4");

	v->ssbDirectionalLight.clear();
	for (auto &entity : ecs::findWith<DirectionalLight>())
	{
		const auto &light = ecs::get<DirectionalLight>(entity);
		glm::vec3 direction = light.direction;

		if (light.shadowCaster)
		{
			float zMax = -numeric_limits<float>::max();
			for (auto &entity : ecs::findWith<Transform, Mesh, BoundingVolume, Occluder>())
			{
				auto &boundingVolume = ecs::get<BoundingVolume>(entity);
				float dist = glm::dot(direction, boundingVolume.sphere.position) + boundingVolume.sphere.radius;

				zMax = max(zMax, dist);
			}

			v->csm.calculateMatrices(light, projection, view, zMax);
		}

		DirectionalLightData *data = v->ssbDirectionalLight.mapData<DirectionalLightData>();
		data->ambient = glm::pow(light.ambient.rgb(), glm::vec3{2.2f});
		data->color = glm::pow(light.color.rgb(), glm::vec3{2.2f});
		data->direction = glm::normalize(glm::mat3{V} * direction);
		data->intensity = light.intensity;
		data->diffuseOnly = light.diffuseOnly;
		data->shadowCaster = light.shadowCaster;
	}
	v->ssbDirectionalLight.upload();

	// CSM

	struct CSMLayerData
	{
		glm::mat4 MVP;
		glm::vec3 centers;
		GLfloat radiuses2;
	};
	struct CSMData
	{
		CSMLayerData layers[fx::CSM::maxCascades];
		GLuint kernelSize;
		GLuint splits;
		GLuint blendCascades;
		GLfloat padding[1];
	};
	static_assert(sizeof(CSMData) % sizeof(glm::vec4) == 0, "Size of CSMData must be padded to size of vec4");

	v->ubCSM.clear();
	{
		assert(fx::CSM::maxCascades >= v->csm.splits && "fx::CSM::maxCascades must be larger or equal to v->csm.splits");

		CSMData *data = v->ubCSM.mapData<CSMData>();
		for (size_t i = 0, c = min(fx::CSM::maxCascades, v->csm.splits); i < c; i++)
		{
			data->layers[i].MVP = v->csm.shadowBiasedVPs[i];
			data->layers[i].centers = v->csm.centers[i];
			data->layers[i].radiuses2 = v->csm.radiuses2[i];
		}
		data->kernelSize = v->csm.kernelSize;
		data->splits = v->csm.splits;
		data->blendCascades = v->csm.blendCascades;
	}
	v->ubCSM.upload();

	/* TODO: create per-tile list of lights */
}

void App::renderShadowMapsPass()
{
	v->profCSM.start();

	// v->csm.renderCascades();

	v->csm.progDepth.use();

	RN_CHECK(glDepthFunc(GL_LESS));

	if (v->toggleShadowFaceCulling.value)
	{
		RN_CHECK(glCullFace(GL_FRONT));
	}

	for (size_t i = 0; i < v->csm.splits; i++)
	{
		v->profCSMLayers[i].start();

		const auto &P = v->csm.Ps[i];
		const auto &V = v->csm.Vs[i];
		const auto VP = P * V;

		v->csm.progDepth.uniform("P", P);
		v->csm.progDepth.uniform("V", V);
		auto locationM = v->csm.progDepth.getUniformLocation("M");

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

			auto &transform = ecs::get<Transform>(entity);

			glm::mat4 M{1.f};
			M = glm::translate(M, transform.translation);
			M = glm::rotate(M, glm::radians(transform.rotation.x), glm::vec3{1.f, 0.f, 0.f});
			M = glm::rotate(M, glm::radians(transform.rotation.y), glm::vec3{0.f, 1.f, 0.f});
			M = glm::rotate(M, glm::radians(transform.rotation.z), glm::vec3{0.f, 0.f, 1.f});
			M = glm::scale(M, transform.scale);

			v->csm.progDepth.var(locationM, M);

			proc::MeshRenderer::render(entity);
		}

		v->profCSMLayers[i].stop();
	}

	if (v->toggleShadowFaceCulling.value)
	{
		RN_CHECK(glCullFace(rn::Default::cullFace));
	}

	RN_CHECK(glDepthFunc(rn::Default::depthFunc));

	v->csm.progDepth.forgo();

	v->profCSM.stop();
}

void App::forwardLightingPass()
{
	const auto &projection = ecs::get<Projection>(v->cameraId);
	const auto &view = ecs::get<View>(v->cameraId);
	const auto &P = projection.matrix;
	const auto &V = view.matrix;
	const auto &invV = view.invMatrix;

	const phs::Frustum frustum{P, V};

	RN_FB_BIND(v->fbScreen);

	v->fbScreen.clear(rn::BUFFER_COLOR);

	RN_CHECK(glDepthMask(GL_FALSE));

	v->progLightingForward.use();
	v->progLightingForward.uniform("P", P);
	v->progLightingForward.uniform("V", V);
	v->progLightingForward.uniform("invV", invV);

	v->progLightingForward.uniform("fbScale", glm::vec2{1.f / v->fbScreen.width, 1.f / v->fbScreen.height});

	size_t unit = 0;
	v->progLightingForward.uniform("texAO", v->ssao.fbAO.color(0)->bind(unit++));
	v->progLightingForward.uniform("texCSMDepths", v->csm.texDepths->bind(unit++));

	auto locationMatDiffuse = v->progLightingForward.getUniformLocation("matDiffuse");
	auto locationMatRoughness = v->progLightingForward.getUniformLocation("matRoughness");
	auto locationM = v->progLightingForward.getUniformLocation("M");

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

		const auto &material = ecs::get<Material>(entity);
		v->progLightingForward.var(locationMatDiffuse, material.diffuse);
		v->progLightingForward.var(locationMatRoughness, material.roughness);

		auto &transform = ecs::get<Transform>(entity);
		glm::mat4 M{1.f};
		M = glm::translate(M, transform.translation);
		M = glm::rotate(M, glm::radians(transform.rotation.x), glm::vec3{1.f, 0.f, 0.f});
		M = glm::rotate(M, glm::radians(transform.rotation.y), glm::vec3{0.f, 1.f, 0.f});
		M = glm::rotate(M, glm::radians(transform.rotation.z), glm::vec3{0.f, 0.f, 1.f});
		M = glm::scale(M, transform.scale);

		v->progLightingForward.var(locationM, M);

		proc::MeshRenderer::render(entity);
	}

	v->progLightingForward.forgo();

	RN_CHECK(glDepthMask(rn::Default::depthMask));
}

void App::wireframePass()
{
	if ( ! v->toggleWireframe.value)
	{
		return;
	}

	const auto &projection = ecs::get<Projection>(v->cameraId);
	const auto &view = ecs::get<View>(v->cameraId);
	const auto &P = projection.matrix;
	const auto &V = view.matrix;

	const phs::Frustum frustum{P, V};

	RN_FB_BIND(v->fbScreen);

	RN_SCOPE_DISABLE(GL_DEPTH_TEST);
	RN_SCOPE_DISABLE(GL_CULL_FACE);
	RN_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

	v->progWireframe.use();
	v->progWireframe.uniform("P", P);
	v->progWireframe.uniform("V", V);
	auto locationM = v->progWireframe.getUniformLocation("M");

	for (auto &entity : ecs::findWith<Transform, Mesh, Material, BoundingVolume>())
	{
		if ( ! proc::FrustumProcess::isVisible(entity, frustum))
		{
			continue;
		}

		auto &boundingVolume = ecs::get<BoundingVolume>(entity);

		glm::mat4 M;
		M = glm::translate(glm::mat4{1.f}, boundingVolume.sphere.position);
		M = glm::scale(M, glm::vec3{boundingVolume.sphere.radius});

		v->progWireframe.var(locationM, M);
		proc::MeshRenderer::render(v->bvSphereId);
	}

	v->progWireframe.forgo();

	RN_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
}

void App::postProcessingPass()
{
	RN_SCOPE_DISABLE(GL_DEPTH_TEST);

	v->progGammaCorrection.use();
	v->progGammaCorrection.uniform("texSource", v->fbScreen.color(0)->bind(0));

	rn::Mesh::quad.render();

	v->progGammaCorrection.forgo();
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

	v->ssao.blur();
}
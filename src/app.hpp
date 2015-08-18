#ifndef APP_HPP
#define APP_HPP

#include <core/ecs/entity.hpp>

#include <core/rn/fb.hpp>
#include <core/rn/tb.hpp>
#include <core/rn/font.hpp>
#include <core/rn/mesh.hpp>
#include <core/rn/prof.hpp>
#include <core/rn/program.hpp>

#include <core/phs/frustum.hpp>

#include <app/fx/ssao.hpp>
#include <app/fx/csm.hpp>
#include <app/scene.hpp>

class App
{
public:
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
	rn::Prof profLighting{"App::profLighting"};
	rn::TB directionalLightData{"App::directionalLightData"};
	rn::TB pointLightData{"App::pointLightData"};
	int directionalLightCount;
	int pointLightCount;

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

	App() {};
	~App() {};

	void init();

	void initProg();
	void initFB();
	void initScene();

	void update();

	void render();

	void zPrefillForwardPass();
	void setupLightsForwardPass();
	void lightingForwardPass();

	void gBufferPass();
	void directionalLightsPass();
	void pointLightsPass();
	void flatLightPass();
	void ssaoPass();
};

#endif
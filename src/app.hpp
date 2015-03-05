#ifndef APP_HPP
#define APP_HPP

#include <core/ecs/entity.hpp>

#include <core/rn/mesh.hpp>
#include <core/rn/program.hpp>

#include <core/rn/fb.hpp>
#include <core/rn/prof.hpp>

#include <app/fx/ssao.hpp>
#include <app/scene.hpp>

class App
{
public:
	ecs::Entity cameraId{};
	ecs::Entity lightIds[10]{};

	rn::Program progGBuffer{};
	rn::Program progShadowMap{};

	rn::Program progDirectionalLight{};
	rn::Program progPointLight{};
	rn::Program progFlatLight{};

	rn::Program progSSAOBlit{};

	rn::Program progFBOBlit{};
	rn::Program progBlurGaussian7{};
	rn::Program progBlurPreview{};
	rn::Program progShadowMapPreview{};
	rn::Program progTexPreview{};

	rn::FB fbGBuffer{"App::fbGBuffer"};
	rn::FB fbShadowMap{"App::fbShadowMap"};
	rn::FB fbShadowMapBlur{"App::fbShadowMapBlur"};

	fx::SSAO ssao{};

	rn::Prof profRender{};
	rn::Prof profGBuffer{};
	rn::Prof profDirectionalLight{};
	rn::Prof profPointLight{};
	rn::Prof profFlatLight{};
	rn::Prof profSSAO{};

	app::Scene scene{};

	App() {};
	~App() {};

	void init();

	void initProg();
	void initFB();
	void initScene();

	void update();

	void render();

	void gBufferPass();
	void directionalLightsPass();
	void pointLightsPass();
	void flatLightPass();

	void ssaoPass();

	glm::mat4 genShadowMap(ecs::Entity lightId);
};

#endif
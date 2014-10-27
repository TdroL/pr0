#ifndef APP_HPP
#define APP_HPP

#include <vector>

#include <core/rn/fbo.hpp>
#include <core/rn/fbocube.hpp>

#include <core/rn/mesh.hpp>
#include <core/rn/program.hpp>

#include <core/ecs/entity.hpp>

class App
{
public:
	ecs::Entity cameraId{};
	ecs::Entity lightIds[10]{};

	rn::Program deferredGBuffer{};
	rn::Program deferredShadowMap{};

	rn::Program deferredDirectionalLight{};
	rn::Program deferredPointLight{};
	rn::Program deferredFlatLight{};

	rn::Program ambientOcclusion{};

	rn::Program simple{};
	rn::Program blurFilter{};
	rn::Program blurPreview{};
	rn::Program shadowMapPreview{};

	rn::Tex2D ssaoNoise{};

	rn::FBO gBuffer{"gBuffer"};
	rn::FBO blurBuffer{"blurBuffer"};
	rn::FBO shadowMapBuffer{"shadowMapBuffer"};
	// rn::FBOCube shadowCubeMapBuffer{"shadowCubeMapBuffer"};

	App() {};
	~App() {};

	void init();

	void update();

	void render();

	void gBufferPass();
	void directionalLightsPass();
	void pointLightsPass();
	void flatLightPass();

	void ssao();

	glm::mat4 genShadowMap(ecs::Entity lightId);
};

#endif
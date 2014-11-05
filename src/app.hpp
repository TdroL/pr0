#ifndef APP_HPP
#define APP_HPP

#include <core/ecs/entity.hpp>

#include <core/rn/mesh.hpp>
#include <core/rn/program.hpp>

#include <core/rn/fbo.hpp>
#include <core/rn/tex2d.hpp>

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

	rn::Program progSSAO{};
	rn::Program progSSAOBlit{};

	rn::Program progFBOBlit{};
	rn::Program progBlurGaussian7{};
	rn::Program progBlurBox4{};
	rn::Program progBlurPreview{};
	rn::Program progShadowMapPreview{};

	rn::FBO fboGBuffer{"gBuffer"};
	rn::FBO fboShadowMapBuffer{"shadowMapBuffer"};
	rn::FBO fboShadowMapBlurBuffer{"fboShadowMapBlurBuffer"};
	rn::FBO fboSSAOBuffer{"fboSSAOBuffer"};
	rn::FBO fboSSAOBlurBuffer{"fboSSAOBlurBuffer"};

	rn::Tex2D texNoise{"SSAO noise", GL_RGB8, GL_LINEAR, GL_REPEAT};

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
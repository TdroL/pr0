#ifndef APP_HPP
#define APP_HPP

#include <vector>

#include <core/gl/fbo.hpp>
#include <core/gl/fbocube.hpp>

#include <core/gl/mesh.hpp>
#include <core/gl/program.hpp>

#include <core/ecs/entity.hpp>

class App
{
public:
	ecs::Entity cameraId{};
	ecs::Entity lightIds[10]{};

	gl::Program deferredGBuffer{};
	gl::Program deferredShadowMap{};
	gl::Program deferredDirectionalLight{};
	gl::Program deferredSkinDirectionalLight{};
	gl::Program deferredPointLight{};
	gl::Program simple{};
	gl::Program blurFilter{};
	gl::Program blurPreview{};
	gl::Program shadowMapPreview{};

	gl::FBO gBuffer{"gBuffer"};
	gl::FBO blurBuffer{"blurBuffer"};
	gl::FBO shadowMapBuffer{"shadowMapBuffer"};
	// gl::FBOCube shadowCubeMapBuffer{"shadowCubeMapBuffer"};

	App() {};
	~App() {};

	void init();

	void update();

	void render();

	void gBufferPass();
	void directionalLightsPass();
	void pointLightsPass();
	void flatLightPass();

	glm::mat4 genShadowMap(ecs::Entity lightId);
};

#endif
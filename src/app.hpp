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
	gl::Program deferredPointLight{};
	gl::Program deferredDirectionalLight{};
	gl::Program simple{};
	gl::Program blur{};
	gl::Program blurPreview{};
	gl::Program shadowmapPreview{};

	gl::FBO gbuffer{"gbuffer"};
	gl::FBO blurfilter{"blurfilter"};
	gl::FBO shadowmap{"shadowmap"};
	gl::FBOCube shadowcubemap{"shadowcubemap"};

	App() {};
	~App() {};

	void init();

	void update();

	void render();

	void gbufferPass();
	void directionalLightsPass();
	void pointLightsPass();
};

#endif
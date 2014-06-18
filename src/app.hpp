#ifndef APP_HPP
#define APP_HPP

#include <vector>

#include <core/gl/fbo.hpp>

#include <core/gl/mesh.hpp>
#include <core/gl/program.hpp>

#include <core/ecs/entity.hpp>

class App
{
public:
	ecs::Entity cameraId{};
	ecs::Entity lightIds[10]{};

	gl::Program deferredGBuffer{};
	gl::Program deferredPointLight{};
	gl::Program deferredDirectionalLight{};
	gl::Program simple{};
	gl::Program fboPreview{};

	gl::FBO gbuffer{"gbuffer"};
	gl::FBO shadowmap{"shadowmap"};
	gl::FBO shadowcubemap{"shadowcubemap"};

	App() {};
	~App() {};

	void init();

	void update();

	void render();
};

#endif
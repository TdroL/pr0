#include "ngn.hpp"

#include "rn.hpp"
#include "rn/prof.hpp"

#include "ngn/key.hpp"
#include "ngn/window.hpp"

#include <iostream>

namespace ngn
{

using namespace std;

double ct = 0.0;
double dt = numeric_limits<double>::epsilon();

util::InitQ & initQ()
{
	static util::InitQ q{"ngn::initQ"};
	return q;
}

void init()
{
	{
		auto err = glfwInit();
		if (err != GL_TRUE)
		{
			throw string{"ngn::init() - glfwInit() - failed to initialize GLFW"};
		}

		glfwSetErrorCallback([](int code, const char *message)
		{
			throw string{"GLFW error [" + to_string(code) + "] - " + string{message}};
		});
	}

	window::create(1600, 900);

#if defined(NGN_USE_GLEW)
	{
		glewExperimental = GL_TRUE;
		auto err = glewInit();
		if (err != GLEW_OK)
		{
			throw string{"ngn::init() - glewInit() - "} + reinterpret_cast<const char *>(glewGetErrorString(err));
		}
	}
#elif (NGN_USE_GL3W)
	if (gl3wInit())
	{
		throw string{"ngn::init() - gl3wInit() - failed to initialize OpenGL"};
	}

	if ( ! gl3wIsSupported(window::contextMajor, window::contextMinor))
	{
		int versionMajor;
		int versionMinor;
		rn::get(GL_MAJOR_VERSION, versionMajor);
		rn::get(GL_MINOR_VERSION, versionMinor);

		throw string{"ngn::init() - gl3wIsSupported(" + to_string(window::contextMajor) + ", " + to_string(window::contextMinor) + ") - OpenGL " + to_string(versionMajor) + "." + to_string(versionMinor)};
	}
#else
	if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
	{
		throw string{"ngn::init() - ogl_LoadFunctions() - failed to initialize OpenGL"};
	}

	if ( ! ogl_IsVersionGEQ(window::contextMajor, window::contextMinor))
	{
		int versionMajor = ogl_GetMinorVersion();
		int versionMinor = ogl_GetMajorVersion();

		throw string{"ngn::init() - ogl_IsVersionGEQ(" + to_string(window::contextMajor) + ", " + to_string(window::contextMinor) + ") - OpenGL " + to_string(versionMajor) + "." + to_string(versionMinor)};
	}
#endif

	UTIL_DEBUG
	{
		clog << "OpenGL info:" << endl;
		clog << rn::getBasicInfo() << endl;
	}

	initQ().run();
}

void deinit()
{
	window::release();

	glfwTerminate();
}

void update()
{
	double t = ngn::time();
	dt = t - ct;
	ct = t;

	glfwPollEvents();

	key::update();
}

void startLoop()
{
	ngn::update();
}

void endLoop()
{
	rn::Prof::swapAll();
	rn::stats.reset();
	ngn::window::swapBuffers();
}

double time()
{
	return glfwGetTime();
}

} // ngn
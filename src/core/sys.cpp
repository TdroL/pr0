#include "sys.hpp"
#include "gl.hpp"
#include "gl/font.hpp"
#include "gl/fbo.hpp"
#include "sys/window.hpp"
#include "sys/key.hpp"

#include <limits>
#include <GLFW/glfw3.h>
#include <GL/glew.h>

namespace sys
{

using namespace std;

double ct = 0.0;
double dt = numeric_limits<double>::epsilon();

void init()
{
	{
		auto err = glfwInit();
		if (err != GL_TRUE)
		{
			throw string{"sys::init() - glfwInit() - failed"};
		}

		glfwSetErrorCallback([](int code, const char *message) {
			throw string{"GLFW error [" + to_string(code) + "] - " + string{message}};
		});
	}

	window::create(1600, 900);

	{
		glewExperimental = GL_TRUE;
		auto err = glewInit();
		if (err != GLEW_OK)
		{
			throw string{"sys::init() - glewInit() - "} + reinterpret_cast<const char *>(glewGetErrorString(err));
		}
	}

	gl::init();

	gl::Font::init();

	gl::FBO::init();

	key::init();
}

void deinit()
{
	window::release();

	glfwTerminate();
}

void update()
{
	double t = sys::time();
	dt = t - ct;
	ct = t;

	glfwPollEvents();

	key::update();
}

void startLoop()
{
	sys::update();
}

void endLoop()
{
	glUseProgram(0);
	gl::stats = {0, 0};
	sys::window::swapBuffers();
}

double time()
{
	return glfwGetTime();
}

} // sys
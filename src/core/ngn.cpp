#include "ngn.hpp"

#include "gl.hpp"

#include <GLFW/glfw3.h>
#include <GL/glew.h>

#include "ngn/window.hpp"
#include "ngn/key.hpp"

#include <iostream>

namespace ngn
{

using namespace std;

util::InitQ initQ{};

double ct = 0.0;
double dt = numeric_limits<double>::epsilon();

void init()
{
	{
		auto err = glfwInit();
		if (err != GL_TRUE)
		{
			throw string{"ngn::init() - glfwInit() - failed to initialize GLFW"};
		}

		glfwSetErrorCallback([](int code, const char *message) {
			throw string{"GLFW error [" + to_string(code) + "] - " + string{message}};
		});
	}

	window::create(1600, 900);

	{
		// glewExperimental = GL_TRUE;
		auto err = glewInit();
		if (err != GLEW_OK)
		{
			throw string{"ngn::init() - glewInit() - "} + reinterpret_cast<const char *>(glewGetErrorString(err));
		}
	}

	window::vsync(1);

	ngn::initQ.run();
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
	glUseProgram(0);
	gl::stats.reset();
	ngn::window::swapBuffers();
}

double time()
{
	return glfwGetTime();
}

} // ngn
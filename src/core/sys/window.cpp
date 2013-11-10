#include "window.hpp"
#include <cassert>

namespace sys
{

namespace window
{

using namespace std;

string title{"pr0"};
GLFWwindow *handler = nullptr;
int width = -1;
int height = -1;

void create(int width, int height)
{
	release();

	window::width = width;
	window::height = height;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	glfwWindowHint(GLFW_DECORATED, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	handler = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

	if ( ! handler)
	{
		throw string{"sys::window::create() - failed to create new window"};
	}

	glfwSetWindowPos(handler, 10, 30);
	glfwMakeContextCurrent(handler);
}

void release()
{
	if (handler != nullptr)
	{
		glfwDestroyWindow(handler);
		handler = nullptr;
	}
}

void switchMode(Mode mode)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (mode == Mode::borderless)
	{
		glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	}
	else
	{
		glfwWindowHint(GLFW_DECORATED, GL_TRUE);
	}

	GLFWmonitor *monitor = nullptr;
	if (mode == Mode::fullscreen)
	{
		monitor = glfwGetPrimaryMonitor();
	}

	if (mode == Mode::windowed)
	{
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	}
	else
	{
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	}

	GLFWwindow *newHandler = glfwCreateWindow(width, height, title.c_str(), monitor, handler);

	if ( ! newHandler)
	{
		throw string{"sys::window::switchMode() - failed to create new window"};
	}

	if (mode == Mode::windowed)
	{
		glfwSetWindowPos(newHandler, 10, 30);
	}
	else if (mode == Mode::borderless)
	{
		glfwSetWindowPos(newHandler, 0, 0);
	}

	glfwMakeContextCurrent(newHandler);

	if (handler != nullptr)
	{
		glfwDestroyWindow(handler);
	}

	handler = newHandler;
}

void setTitle(const string &title)
{
	assert(handler != nullptr);
	window::title = title;
	glfwSetWindowTitle(handler, title.c_str());
}

void setTitle(string &&title)
{
	assert(handler != nullptr);
	window::title = move(title);
	glfwSetWindowTitle(handler, title.c_str());
}

void vsync(int mode)
{
	glfwSwapInterval(mode);
}

bool shouldClose()
{
	assert(handler != nullptr);
	return glfwWindowShouldClose(handler);
}

void close()
{
	assert(handler != nullptr);
	glfwSetWindowShouldClose(handler, GL_TRUE);
}

void swapBuffers()
{
	assert(handler != nullptr);
	glfwSwapBuffers(handler);
}

} // window

} // sys
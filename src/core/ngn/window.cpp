#include "window.hpp"
#include <cassert>
#include <iostream>

namespace ngn
{

namespace window
{

using namespace std;

string title{"pr0"};
GLFWwindow *handler = nullptr;
int width = -1;
int height = -1;

namespace
{
	void errorCallback(int error, const char* description)
	{
		cerr << "[GLFW error] ";

		switch (error)
		{
			case GLFW_NOT_INITIALIZED:
			{
				cerr << "GLFW_NOT_INITIALIZED";
				break;
			}
			case GLFW_NO_CURRENT_CONTEXT:
			{
				cerr << "GLFW_NO_CURRENT_CONTEXT";
				break;
			}
			case GLFW_INVALID_ENUM:
			{
				cerr << "GLFW_INVALID_ENUM";
				break;
			}
			case GLFW_INVALID_VALUE:
			{
				cerr << "GLFW_INVALID_VALUE";
				break;
			}
			case GLFW_OUT_OF_MEMORY:
			{
				cerr << "GLFW_OUT_OF_MEMORY";
				break;
			}
			case GLFW_API_UNAVAILABLE:
			{
				cerr << "GLFW_API_UNAVAILABLE";
				break;
			}
			case GLFW_VERSION_UNAVAILABLE:
			{
				cerr << "GLFW_VERSION_UNAVAILABLE";
				break;
			}
			case GLFW_PLATFORM_ERROR:
			{
				cerr << "GLFW_PLATFORM_ERROR";
				break;
			}
			case GLFW_FORMAT_UNAVAILABLE:
			{
				cerr << "GLFW_FORMAT_UNAVAILABLE";
				break;
			}
			default:
			{
				cerr << error;
			}
		}

		cerr << ": " << description << endl;
	}
}

void resetHints()
{
	glfwDefaultWindowHints();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // GLEW in core profile throws GL_INVALID_ENUMs

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
}

void create(int width, int height)
{
	release();

	window::width = width;
	window::height = height;

	resetHints();

	glfwWindowHint(GLFW_DECORATED, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	handler = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

	if ( ! handler)
	{
		throw string{"ngn::window::create() - failed to create new window"};
	}

	glfwSetWindowPos(handler, 10, 30);
	glfwMakeContextCurrent(handler);

	glfwSetErrorCallback(errorCallback);
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
	resetHints();

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
		throw string{"ngn::window::switchMode() - failed to create new window"};
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

} // ngn
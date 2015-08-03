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
bool linkInternalResolution = true;
int internalWidth = 1920;
int internalHeight = 1080;
const int contextMajor = 3;
const int contextMinor = 3;

Mode currentMode = Mode::windowed;
int currentVsync = 0;

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

	void resizeCallback(GLFWwindow*, int width, int height)
	{
		window::width = width;
		window::height = height;

		if (linkInternalResolution)
		{
			window::internalWidth = width;
			window::internalHeight = height;
		}

		event::emit(WindowResizeEvent{width, height});
	}
}

void resetHints()
{
	glfwDefaultWindowHints();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, contextMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, contextMinor);

#ifdef NGN_USE_GLEW
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); // GLEW in core profile throws GL_INVALID_ENUMs
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	glfwWindowHint(GLFW_REFRESH_RATE, GL_DONT_CARE);

	glfwWindowHint(GLFW_RED_BITS, GL_DONT_CARE);
	glfwWindowHint(GLFW_GREEN_BITS, GL_DONT_CARE);
	glfwWindowHint(GLFW_BLUE_BITS, GL_DONT_CARE);
}

void create(int width, int height)
{
	release();

	window::width = width;
	window::height = height;

	if (linkInternalResolution)
	{
		window::internalWidth = width;
		window::internalHeight = height;
	}

	switchMode(currentMode, currentVsync);
}

void release()
{
	if (handler != nullptr)
	{
		glfwDestroyWindow(handler);
		handler = nullptr;
	}
}

void switchMode(Mode mode, int vsync)
{
	resetHints();

	GLFWmonitor *monitor = nullptr;

	switch (mode)
	{
		case Mode::borderless:
			glfwWindowHint(GLFW_DECORATED, GL_FALSE);
			glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		break;

		case Mode::windowed:
			glfwWindowHint(GLFW_DECORATED, GL_TRUE);
			glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
		break;

		case Mode::fullscreen:
			if (handler && mode == currentMode)
			{
				// Create temporary window, so switching vsync in fullscreen wont end as borderless mode
				GLFWwindow *tempHandler = glfwCreateWindow(width, height, "Temporary window", nullptr, handler);

				if ( ! tempHandler)
				{
					throw string{"ngn::window::switchMode() - failed to create temporary window"};
				}

				glfwMakeContextCurrent(tempHandler);

				glfwDestroyWindow(handler);

				handler = tempHandler;
			}

			glfwWindowHint(GLFW_DECORATED, GL_DONT_CARE);
			glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
			monitor = glfwGetPrimaryMonitor();

			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		break;
	}

	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

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

	glfwSetErrorCallback(errorCallback);
	glfwSetWindowSizeCallback(newHandler, resizeCallback);

	glfwSwapInterval(vsync);

	handler = newHandler;

	currentMode = mode;
	currentVsync = vsync;
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
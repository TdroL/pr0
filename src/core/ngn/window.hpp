#ifndef NGN_WINDOW_HPP
#define NGN_WINDOW_HPP

#include "../event.hpp"
#include <GLFW/glfw3.h>
#include <string>

namespace ngn
{

namespace window
{

/* #Events */

struct WindowResizeEvent : public event::Event<WindowResizeEvent>
{
	const int width;
	const int height;
	WindowResizeEvent(int width, int height) : width{width}, height{height} {}
};

/* /Events */

enum Mode
{
	windowed,
	borderless,
	fullscreen
};

extern std::string title;
extern GLFWwindow *handler;
extern GLFWwindow *initial;
extern int width;
extern int height;
extern const int contextMajor;
extern const int contextMinor;

extern Mode currentMode;
extern int currentVsync;

void create(int width, int height);

void release();

void switchMode(Mode mode, int vsync);

void setTitle(const std::string &title);
void setTitle(std::string &&title);

bool shouldClose();
void close();

void swapBuffers();

} // window

} // ngn

#endif
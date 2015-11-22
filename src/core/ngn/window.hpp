#pragma once

#include "../event.hpp"

#include <string>

struct GLFWwindow;

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
extern int width;
extern int height;
extern bool linkInternalResolution;
extern int internalWidth;
extern int internalHeight;
extern const int contextMajor;
extern const int contextMinor;

extern Mode currentMode;
extern int currentVsync;

void create(int width, int height);

void release();

void switchMode(Mode mode, int vsync);

void setTitle(const std::string &title);
void setTitle(std::string &&title);

GLFWwindow * getHandler();

bool shouldClose();
void close();

void swapBuffers();

} // window

} // ngn
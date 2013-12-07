#ifndef SYS_WINDOW_HPP
#define SYS_WINDOW_HPP

#include <GLFW/glfw3.h>
#include <string>

namespace sys
{

namespace window
{

extern std::string title;
extern GLFWwindow *handler;
extern GLFWwindow *initial;
extern int width;
extern int height;

enum Mode
{
	windowed,
	borderless,
	fullscreen
};

void create(int width, int height);

void release();

void switchMode(Mode mode);

void setTitle(const std::string &title);
void setTitle(std::string &&title);

void vsync(int mode);

bool shouldClose();
void close();

void swapBuffers();

} // window

} // sys

#endif
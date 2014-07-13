#ifndef NGN_WINDOW_HPP
#define NGN_WINDOW_HPP

#include <GLFW/glfw3.h>
#include <string>

namespace ngn
{

namespace window
{

extern std::string title;
extern GLFWwindow *handler;
extern GLFWwindow *initial;
extern int width;
extern int height;
extern const int contextMajor;
extern const int contextMinor;

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

} // ngn

#endif
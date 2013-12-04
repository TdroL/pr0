#include "key.hpp"
#include "window.hpp"
#include <vector>
#include <string>
#include <ranges>
#include <GLFW/glfw3.h>

namespace sys
{

namespace key
{

using namespace std;

vector<bool> keyIsHit{};
vector<bool> keyIsPressed{};
vector<bool> keyWasPressed{};

void init()
{
	keyIsHit.resize(GLFW_KEY_LAST, false);
	keyIsPressed.resize(GLFW_KEY_LAST, false);
	keyWasPressed.resize(GLFW_KEY_LAST, false);

	glfwSetInputMode(sys::window::handler, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(sys::window::handler, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
}

bool hit(size_t key)
{
	if (key >= keyIsHit.size())
	{
		throw string{"sys::key::isHit() - invalid key " + to_string(key)};
	}
	return keyIsHit[key];
}

bool pressed(size_t key)
{
	if (key >= keyIsPressed.size())
	{
		throw string{"sys::key::isPressed() - invalid key " + to_string(key)};
	}
	return keyIsPressed[key];
}

bool wasPressed(size_t key)
{
	if (key >= keyWasPressed.size())
	{
		throw string{"sys::key::wasPressed() - invalid key " + to_string(key)};
	}
	return keyWasPressed[key];
}

void update()
{
	swap(keyWasPressed, keyIsPressed);

	for (size_t i : range(keyIsPressed.size()))
	{
		keyIsPressed[i] = (glfwGetKey(sys::window::handler, i) == GLFW_PRESS);
		keyIsHit[i] = (keyIsPressed[i] && ! keyWasPressed[i]);
	}

	// overwrite lowercase keys with values from uppercase keys
	for (ptrdiff_t c : range('a', 'z'))
	{
		ptrdiff_t C = toupper(c);
		keyIsPressed[c] = keyIsPressed[C];
		keyIsHit[c] = keyIsHit[C];
	}
}

} // key

} // sys
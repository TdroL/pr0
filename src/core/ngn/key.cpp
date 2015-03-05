#include "key.hpp"
#include "../ngn.hpp"
#include "../util/str.hpp"
#include "window.hpp"
#include <vector>
#include <string>
#include <ranges>

namespace ngn
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

	glfwSetInputMode(ngn::window::handler, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(ngn::window::handler, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
}

bool hit(size_t key)
{
	if (key >= keyIsHit.size())
	{
		throw string{"ngn::key::isHit() - invalid key " + to_string(key)};
	}

	return keyIsHit[key];
}

bool pressed(size_t key)
{
	if (key >= keyIsPressed.size())
	{
		throw string{"ngn::key::isPressed() - invalid key " + to_string(key)};
	}

	return keyIsPressed[key];
}

bool wasPressed(size_t key)
{
	if (key >= keyWasPressed.size())
	{
		throw string{"ngn::key::wasPressed() - invalid key " + to_string(key)};
	}

	return keyWasPressed[key];
}

size_t getKey(std::string key)
{
	if (key.empty())
	{
		return 0;
	}

	util::str::uppercase(key);

	if (key.length() == 1)
	{
		return static_cast<size_t>(key[0]);
	}

	if (key == "SPACE")       { return KEY_SPACE; }
	if (key == "APOSTROPHE")  { return KEY_APOSTROPHE; }
	if (key == "COMMA")       { return KEY_COMMA; }
	if (key == "MINUS")       { return KEY_MINUS; }
	if (key == "PERIOD")      { return KEY_PERIOD; }
	if (key == "SLASH")       { return KEY_SLASH; }
	if (key == "SEMICOLON")   { return KEY_SEMICOLON; }
	if (key == "EQUAL")       { return KEY_EQUAL; }
	if (key == "LBRACKET")    { return KEY_LBRACKET; }
	if (key == "BACKSLASH")   { return KEY_BACKSLASH; }
	if (key == "RBRACKET")    { return KEY_RBRACKET; }
	if (key == "GRAVEACCENT") { return KEY_GRAVE_ACCENT; }
	if (key == "WORLD1")      { return KEY_WORLD_1; }
	if (key == "WORLD2")      { return KEY_WORLD_2; }
	if (key == "ESCAPE")      { return KEY_ESCAPE; }
	if (key == "ENTER")       { return KEY_ENTER; }
	if (key == "TAB")         { return KEY_TAB; }
	if (key == "BACKSPACE")   { return KEY_BACKSPACE; }
	if (key == "INSERT")      { return KEY_INSERT; }
	if (key == "DELETE")      { return KEY_DELETE; }
	if (key == "RIGHT")       { return KEY_RIGHT; }
	if (key == "LEFT")        { return KEY_LEFT; }
	if (key == "DOWN")        { return KEY_DOWN; }
	if (key == "UP")          { return KEY_UP; }
	if (key == "PAGEUP")      { return KEY_PAGE_UP; }
	if (key == "PAGEDOWN")    { return KEY_PAGE_DOWN; }
	if (key == "HOME")        { return KEY_HOME; }
	if (key == "END")         { return KEY_END; }
	if (key == "CAPSLOCK")    { return KEY_CAPS_LOCK; }
	if (key == "SCROLLLOCK")  { return KEY_SCROLL_LOCK; }
	if (key == "NUMLOCK")     { return KEY_NUM_LOCK; }
	if (key == "PRINTSCREEN") { return KEY_PRINT_SCREEN; }
	if (key == "PAUSE")       { return KEY_PAUSE; }
	if (key == "F1")          { return KEY_F1; }
	if (key == "F2")          { return KEY_F2; }
	if (key == "F3")          { return KEY_F3; }
	if (key == "F4")          { return KEY_F4; }
	if (key == "F5")          { return KEY_F5; }
	if (key == "F6")          { return KEY_F6; }
	if (key == "F7")          { return KEY_F7; }
	if (key == "F8")          { return KEY_F8; }
	if (key == "F9")          { return KEY_F9; }
	if (key == "F10")         { return KEY_F10; }
	if (key == "F11")         { return KEY_F11; }
	if (key == "F12")         { return KEY_F12; }
	if (key == "F13")         { return KEY_F13; }
	if (key == "F14")         { return KEY_F14; }
	if (key == "F15")         { return KEY_F15; }
	if (key == "F16")         { return KEY_F16; }
	if (key == "F17")         { return KEY_F17; }
	if (key == "F18")         { return KEY_F18; }
	if (key == "F19")         { return KEY_F19; }
	if (key == "F20")         { return KEY_F20; }
	if (key == "F21")         { return KEY_F21; }
	if (key == "F22")         { return KEY_F22; }
	if (key == "F23")         { return KEY_F23; }
	if (key == "F24")         { return KEY_F24; }
	if (key == "F25")         { return KEY_F25; }
	if (key == "KP0")         { return KEY_KP_0; }
	if (key == "KP1")         { return KEY_KP_1; }
	if (key == "KP2")         { return KEY_KP_2; }
	if (key == "KP3")         { return KEY_KP_3; }
	if (key == "KP4")         { return KEY_KP_4; }
	if (key == "KP5")         { return KEY_KP_5; }
	if (key == "KP6")         { return KEY_KP_6; }
	if (key == "KP7")         { return KEY_KP_7; }
	if (key == "KP8")         { return KEY_KP_8; }
	if (key == "KP9")         { return KEY_KP_9; }
	if (key == "KPDECIMAL")   { return KEY_KP_DECIMAL; }
	if (key == "KPDIVIDE")    { return KEY_KP_DIVIDE; }
	if (key == "KPMULTIPLY")  { return KEY_KP_MULTIPLY; }
	if (key == "KPSUBTRACT")  { return KEY_KP_SUBTRACT; }
	if (key == "KPADD")       { return KEY_KP_ADD; }
	if (key == "KPENTER")     { return KEY_KP_ENTER; }
	if (key == "KPEQUAL")     { return KEY_KP_EQUAL; }
	if (key == "LSHIFT")      { return KEY_LSHIFT; }
	if (key == "CTRL")        { return KEY_CTRL; }
	if (key == "LCTRL")       { return KEY_LCTRL; }
	if (key == "LALT")        { return KEY_LALT; }
	if (key == "LSUPER")      { return KEY_LSUPER; }
	if (key == "RSHIFT")      { return KEY_RSHIFT; }
	if (key == "RCTRL")       { return KEY_RCTRL; }
	if (key == "RALT")        { return KEY_RALT; }
	if (key == "RSUPER")      { return KEY_RSUPER; }
	if (key == "MENU")        { return KEY_MENU; }

	return 0;
}

void update()
{
	swap(keyWasPressed, keyIsPressed);

	for (size_t i : range(keyIsPressed.size()))
	{
		keyIsPressed[i] = (glfwGetKey(ngn::window::handler, i) == GLFW_PRESS);
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

namespace
{
	const util::InitQAttacher attach(ngn::initQ(), []
	{
		key::init();
	});
}

} // ngn
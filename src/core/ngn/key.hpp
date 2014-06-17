#ifndef NGN_KEY_HPP
#define NGN_KEY_HPP

#include <cctype>

namespace ngn
{

namespace key
{
void init();

bool hit(size_t key);
bool pressed(size_t key);
bool wasPressed(size_t key);

void update();

#define KEY_SPACE        GLFW_KEY_SPACE
#define KEY_SPECIAL      GLFW_KEY_SPECIAL
#define KEY_ESC          GLFW_KEY_ESCAPE
#define KEY_F1           GLFW_KEY_F1
#define KEY_F2           GLFW_KEY_F2
#define KEY_F3           GLFW_KEY_F3
#define KEY_F4           GLFW_KEY_F4
#define KEY_F5           GLFW_KEY_F5
#define KEY_F6           GLFW_KEY_F6
#define KEY_F7           GLFW_KEY_F7
#define KEY_F8           GLFW_KEY_F8
#define KEY_F9           GLFW_KEY_F9
#define KEY_F10          GLFW_KEY_F10
#define KEY_F11          GLFW_KEY_F11
#define KEY_F12          GLFW_KEY_F12
#define KEY_F13          GLFW_KEY_F13
#define KEY_F14          GLFW_KEY_F14
#define KEY_F15          GLFW_KEY_F15
#define KEY_F16          GLFW_KEY_F16
#define KEY_F17          GLFW_KEY_F17
#define KEY_F18          GLFW_KEY_F18
#define KEY_F19          GLFW_KEY_F19
#define KEY_F20          GLFW_KEY_F20
#define KEY_F21          GLFW_KEY_F21
#define KEY_F22          GLFW_KEY_F22
#define KEY_F23          GLFW_KEY_F23
#define KEY_F24          GLFW_KEY_F24
#define KEY_F25          GLFW_KEY_F25
#define KEY_UP           GLFW_KEY_UP
#define KEY_DOWN         GLFW_KEY_DOWN
#define KEY_LEFT         GLFW_KEY_LEFT
#define KEY_RIGHT        GLFW_KEY_RIGHT
#define KEY_LSHIFT       GLFW_KEY_LEFT_SHIFT
#define KEY_RSHIFT       GLFW_KEY_RIGHT_SHIFT
#define KEY_SHIFT        GLFW_KEY_LEFT_SHIFT
#define KEY_LCTRL        GLFW_KEY_LEFT_CONTROL
#define KEY_RCTRL        GLFW_KEY_RIGHT_CONTROL
#define KEY_CTRL         GLFW_KEY_LEFT_CONTROL
#define KEY_LALT         GLFW_KEY_LEFT_ALT
#define KEY_RALT         GLFW_KEY_RIGHT_ALT
#define KEY_TAB          GLFW_KEY_TAB
#define KEY_ENTER        GLFW_KEY_ENTER
#define KEY_BACKSPACE    GLFW_KEY_BACKSPACE
#define KEY_INSERT       GLFW_KEY_INSERT
#define KEY_DEL          GLFW_KEY_DELETE
#define KEY_PAGEUP       GLFW_KEY_PAGE_UP
#define KEY_PAGEDOWN     GLFW_KEY_PAGE_DOWN
#define KEY_HOME         GLFW_KEY_HOME
#define KEY_END          GLFW_KEY_END
#define KEY_KP_0         GLFW_KEY_KP_0
#define KEY_KP_1         GLFW_KEY_KP_1
#define KEY_KP_2         GLFW_KEY_KP_2
#define KEY_KP_3         GLFW_KEY_KP_3
#define KEY_KP_4         GLFW_KEY_KP_4
#define KEY_KP_5         GLFW_KEY_KP_5
#define KEY_KP_6         GLFW_KEY_KP_6
#define KEY_KP_7         GLFW_KEY_KP_7
#define KEY_KP_8         GLFW_KEY_KP_8
#define KEY_KP_9         GLFW_KEY_KP_9
#define KEY_KP_DIVIDE    GLFW_KEY_KP_DIVIDE
#define KEY_KP_MULTIPLY  GLFW_KEY_KP_MULTIPLY
#define KEY_KP_SUBTRACT  GLFW_KEY_KP_SUBTRACT
#define KEY_KP_ADD       GLFW_KEY_KP_ADD
#define KEY_KP_DECIMAL   GLFW_KEY_KP_DECIMAL
#define KEY_KP_EQUAL     GLFW_KEY_KP_EQUAL
#define KEY_KP_ENTER     GLFW_KEY_KP_ENTER
#define KEY_NUM_LOCK     GLFW_KEY_NUM_LOCK
#define KEY_CAPS_LOCK    GLFW_KEY_CAPS_LOCK
#define KEY_SCROLL_LOCK  GLFW_KEY_SCROLL_LOCK
#define KEY_PAUSE        GLFW_KEY_PAUSE
#define KEY_LSUPER       GLFW_KEY_LEFT_SUPER
#define KEY_RSUPER       GLFW_KEY_RIGHT_SUPER
#define KEY_MENU         GLFW_KEY_MENU
#define KEY_LAST         GLFW_KEY_LAST

} // key

} // ngn

#endif
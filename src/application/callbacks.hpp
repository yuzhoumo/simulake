#ifndef CALLBACKS_HPP
#define CALLBACKS_HPP

#include "graphics.hpp"

namespace simulake {
namespace callbacks {

void error(int errorcode, const char *description);

void key(GLFWwindow* window, int key, int scancode, int action, int mods);

void cursor_pos(GLFWwindow* window, double xpos, double ypos);

void scroll(GLFWwindow* window, double xoffset, double yoffset);

void framebuffer_size(GLFWwindow* window, int width, int height);

} /* namespace callbacks */
} /* namespace simulake */

#endif

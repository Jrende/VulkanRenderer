#include "Input.hpp"
#include <iostream>

namespace Input {
  namespace {
    std::vector<std::function<void(double, double)>> scroll_handlers;

    std::map<int, ActionHandler> handlers;
    std::set<int> currentKeys;
    double mouseX;
    double mouseY;
    double deltaX;
    double deltaY;
    int mods;
    bool mouseLocked;
    GLFWwindow* window;
    void handle_key(int key) {
      if(handlers.count(key) > 0) {
	handlers[key]();
      }
    }
  }

  void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Input::mods = mods;
    if(action == GLFW_PRESS) {
      if (key == GLFW_KEY_ESCAPE) {
	glfwSetWindowShouldClose(window, GL_TRUE);
      }
      handle_key(key);
      if(handlers.count(key) > 0 && Actions::is_repeatable(key)) {
	currentKeys.insert(key);
      }
    } else if(action == GLFW_RELEASE) {
      currentKeys.erase(key);
    }
  }

  void handle_input() {
    for(int key: currentKeys) {
      handle_key(key);
    }
  }

  void on(int key, const std::function<void()>& handler) {
    on(key, handler, false);
  }

  void on(int key, const std::function<void()>& handler, bool repeat) {
    handlers[key] = handler; 
    if(repeat) {
      Actions::set_repeatable(key);
    }
  }
  
  void on_scroll(std::function<void(double, double)> handler) {
    scroll_handlers.push_back(handler);
  }

  void scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
    for(const auto& handler: scroll_handlers) {
      handler(x_offset, y_offset);
    }
  }

  void cursor_pos_callback(GLFWwindow* window, double x, double y) {
    Input::deltaX = mouseX - x;
    Input::deltaY = mouseY - y;
    Input::mouseX = x;
    Input::mouseY = y;
  }

  double get_mouse_x() {
    return Input::mouseX;
  }
  double get_mouse_y() {
    return Input::mouseY;
  }
  double get_mouse_dx() {
    return Input::deltaX;
  }
  double get_mouse_dy() {
    return Input::deltaY;
  }

  void reset_delta() {
    Input::deltaX = 0;
    Input::deltaY = 0;
  }

  bool is_key_down(int key) {
      return currentKeys.count(key) != 0;
  }

  bool is_mouse_locked() {
    return mouseLocked;
  }

  void lock_mouse() {
    if(mouseLocked) {
      glfwSetInputMode(Input::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(Input::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    mouseLocked = !mouseLocked;
  }

  void set_active_window(GLFWwindow* window) {
    glfwSetCursorPosCallback(window, Input::cursor_pos_callback);
    glfwSetKeyCallback(window, Input::key_callback);
    glfwSetScrollCallback(window, Input::scroll_callback);
    Input::window = window;
  }

  bool is_shift_down() {
    return (mods & 0x0001) > 0;
  }

  bool is_control_down() {
    return (mods & 0x0002) > 0;
  }

  bool is_alt_down() {
    return (mods & 0x0004) > 0;
  }

  bool is_super_down() {
    return (mods & 0x0008) > 0;
  }

}

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "input/Actions.hpp"
#include "input/Input.hpp"

#include <iostream>
#include "VulkanTestApp.hpp"
#include <chrono>
#include <iomanip>

inline double calcFPS(double theTimeInterval = 1.0) {
  static double t0Value       = glfwGetTime(); // Set the initial time to now
  static int    fpsFrameCount = 0;             // Set the initial FPS frame count to 0
  static double fps           = 0.0;           // Set the initial FPS value to 0.0

  // Get the current time in seconds since the program started (non-static, so executed every time)
  double currentTime = glfwGetTime();

  // Ensure the time interval between FPS checks is sane (low cap = 0.1s, high-cap = 10.0s)
  // Negative numbers are invalid, 10 fps checks per second at most, 1 every 10 secs at least.
  if (theTimeInterval < 0.1) {
    theTimeInterval = 0.1;
  }
  if (theTimeInterval > 10.0) {
    theTimeInterval = 10.0;
  }

  // Calculate and display the FPS every specified time interval
  if ((currentTime - t0Value) > theTimeInterval) {
    // Calculate the FPS as the number of frames divided by the interval in seconds
    fps = (double)fpsFrameCount / (currentTime - t0Value);

    std::stringstream str;
    str << std::fixed << std::setprecision(1) << fps;
    std::cout << "FPS: " << str.str() << '\n';
    // Reset the FPS frame counter and set the initial time to be now
    fpsFrameCount = 0;
    t0Value = glfwGetTime();

  } else {
    // FPS calculation time interval hasn't elapsed yet? Simply increment the FPS frame counter
    fpsFrameCount++;
  }

  // Return the current FPS - doesn't have to be used if you don't want it!
  return fps;
}

int main() {
  glfwInit();

  if (!glfwVulkanSupported()) {
    std::cout << "This machine does not support vulkan\n";
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

  std::cout << extensionCount << " extensions supported" << std::endl;

  Input::set_active_window(window);
  Input::on(GLFW_KEY_ESCAPE, [&window]() {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      });

  VulkanTestApp vkApp;
  vkApp.init_vulkan(window);
  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    vkApp.draw_frame();
    calcFPS();

    glfwSwapBuffers(window);
  }
  vkApp.cleanup();
  glfwDestroyWindow(window);

  glfwTerminate();

  return 0;
}

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "input/Actions.hpp"
#include "input/Input.hpp"

#include <iostream>
#include "VulkanTestApp.hpp"

double clockToMilliseconds(clock_t ticks){
  return (ticks/(double)CLOCKS_PER_SEC)*1000.0;
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

  clock_t deltaTime = 0;
  unsigned int frames = 0;
  double  frameRate = 30;
  double  averageFrameTimeMilliseconds = 33.333;

  VulkanTestApp vkApp;
  vkApp.init_vulkan(window);
  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    clock_t beginFrame = clock();
    vkApp.draw_frame();
    clock_t endFrame = clock();

    deltaTime += endFrame - beginFrame;
    frames ++;

    if(clockToMilliseconds(deltaTime)>1000.0) {
      frameRate = (double)frames*0.5 + frameRate*0.5; //more stable
      frames = 0;
      deltaTime -= CLOCKS_PER_SEC;
      averageFrameTimeMilliseconds = (frameRate==0?0.001:frameRate);

      std::cout << "CPU time was: " << averageFrameTimeMilliseconds << '\n';
    }

    glfwSwapBuffers(window);
  }
  vkApp.cleanup();
  glfwDestroyWindow(window);

  glfwTerminate();

  return 0;
}

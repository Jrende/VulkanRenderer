#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "input/Actions.hpp"
#include "input/Input.hpp"

#include <iostream>
#include "VulkanTestApp.hpp"

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
    vkApp.initVulkan();
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        vkApp.mainLoop();
        glfwSwapBuffers(window);
    }
    vkApp.cleanup();
    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include "QueueFamilyIndices.hpp"
#include <string>

class VulkanTestApp {
  private:
  VkDebugReportCallbackEXT callback;
  const bool enableValidationLayers = true;
  const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
  };
  GLFWwindow* window;
  QueueFamilyIndices queueFamilyIndices{};
  vk::Instance instance;
  vk::Device device;
  vk::Queue graphicsQueue;
  vk::SurfaceKHR surface;

  void setupDebugCallback();
  void createLogicalDevice(vk::PhysicalDevice& physicalDevice);
  void create_instance();
  void create_surface();
  void enumerate_physical_devices();
  public:
    void mainLoop();
    void initVulkan(GLFWwindow* window);
    void cleanup();
};

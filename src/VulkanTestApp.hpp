#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include "QueueFamilyIndices.hpp"
#include <string>
#include <array>

class VulkanTestApp {
  private:
  static const int NUM_FRAME_DATA = 2;
  VkDebugReportCallbackEXT callback;
  const bool enableValidationLayers = true;
  const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
  };
  const std::vector<const char*> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };
  int width, height;
  GLFWwindow* window;
  QueueFamilyIndices queueFamilyIndices{};
  vk::Instance instance;
  vk::Device device;
  vk::Queue graphicsQueue;
  vk::SurfaceKHR surface;
  vk::PhysicalDevice physical_device;
  std::array<vk::Semaphore, NUM_FRAME_DATA> aquire_semaphores;
  std::array<vk::Semaphore, NUM_FRAME_DATA> render_complete_semaphores;
  vk::CommandPool command_pool;
  vk::CommandBuffer command_buffer;
  std::array<vk::Fence, NUM_FRAME_DATA> command_buffer_fences;
  vk::SwapchainKHR swapchain;
  std::vector<vk::Image> swapchain_images;
  vk::Format swapchain_image_format;
  vk::Extent2D swapchain_extent;
  std::vector<vk::ImageView> swapchain_image_views;

  void setupDebugCallback();
  void create_logical_device();
  void create_instance();
  void create_surface();
  void select_physical_device();
  void create_semaphores();
  void create_command_pool();
  void create_command_buffer();
  void create_swapchain();
  void create_image_views();
  void create_graphics_pipeline();

  public:
    void mainLoop();
    void initVulkan(GLFWwindow* window);
    void cleanup();
};

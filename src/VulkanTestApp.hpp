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
  vk::Queue graphics_queue;
  vk::Queue present_queue;
  vk::SurfaceKHR surface;
  vk::PhysicalDevice physical_device;
  vk::SwapchainKHR swapchain;
  std::vector<vk::Image> swapchain_images;
  vk::Format swapchain_image_format;
  vk::Extent2D swapchain_extent;
  std::vector<vk::ImageView> swapchain_image_views;
  vk::PipelineLayout pipeline_layout;
  vk::RenderPass render_pass;
  vk::Pipeline graphics_pipeline;
  std::vector<vk::Framebuffer> swapchain_framebuffers;
  vk::CommandPool command_pool;
  std::vector<vk::CommandBuffer> command_buffers;
  vk::Semaphore image_available_semaphore;
  vk::Semaphore render_finished_semaphore;

  void setupDebugCallback();
  void create_logical_device();
  void create_instance();
  void create_surface();
  void select_physical_device();
  void create_command_buffers();
  void create_semaphores();
  void create_swapchain();
  void create_image_views();
  void create_render_pass();
  void create_graphics_pipeline();
  void create_frame_buffers();
  void create_command_pool();

  public:
    void draw_frame();
    void init_vulkan(GLFWwindow* window);
    void cleanup();
};

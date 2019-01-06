#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <string>
#include <array>
#include <glm/glm.hpp>
#include "QueueFamilyIndices.hpp"
#include "Vertex.hpp"

class VulkanTestApp {
  private:
  static const int MAX_FRAMES_IN_FLIGHT = 2;
  VkDebugReportCallbackEXT callback;
  const bool enableValidationLayers = true;
  const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
  };
  const std::vector<const char*> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };
  int current_frame = 0;
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
  std::vector<vk::Semaphore> image_available_semaphores;
  std::vector<vk::Semaphore> render_finished_semaphores;
  std::vector<vk::Fence> in_flight_fences;
  vk::Buffer model_buffer;
  vk::DeviceMemory model_buffer_memory;

  std::vector<Vertex> vertices = {{
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
    {{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}},
    {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
  }};
  std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

  void setupDebugCallback();
  void create_logical_device();
  void create_instance();
  void create_surface();
  void select_physical_device();
  void create_model_buffer();
  void create_command_buffers();
  void create_semaphores();
  void create_swapchain();
  void create_image_views();
  void create_render_pass();
  void create_graphics_pipeline();
  void create_frame_buffers();
  void create_command_pool();

  void create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& buffer_memory);
  void copy_buffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size);

  public:
    void draw_frame();
    void init_vulkan(GLFWwindow* window);
    void cleanup();
};

#include "VulkanTestApp.hpp"
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include "Validation.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"
#include "SwapChainSupportDetails.hpp"
#include "Shader.hpp"

void VulkanTestApp::create_instance() {
  vk::ApplicationInfo appInfo{
    "Hello Triangle",
    VK_MAKE_VERSION(1, 0, 0),
    "No Engine",
    VK_MAKE_VERSION(1, 0, 0),
    VK_API_VERSION_1_0
  };

  vk::InstanceCreateInfo createInfo{};
  createInfo.setPApplicationInfo(&appInfo);
  if (enableValidationLayers) {
    createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
    createInfo.setPpEnabledLayerNames(validationLayers.data());
  } else {
    createInfo.setEnabledLayerCount(0);
  }
  auto extensions = jar::validation::get_required_extensions(enableValidationLayers);
  std::cout << "Required instancce extensions: \n";
  for(const auto& e: extensions) {
    std::cout << "\t" << e << '\n';
  }
  
  createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
  createInfo.setPpEnabledExtensionNames(extensions.data());

  if(enableValidationLayers) {
    createInfo.setEnabledLayerCount(validationLayers.size());
    createInfo.setPpEnabledLayerNames(validationLayers.data());
  }

  if (vk::createInstance(&createInfo, nullptr, &(this->instance)) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create instance!");
  }

  if(enableValidationLayers) {
    setupDebugCallback();
  }
}

void VulkanTestApp::create_surface() {
  VkSurfaceKHR vk_surface{};
  if (glfwCreateWindowSurface(instance, window, nullptr, &vk_surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
  this->surface = vk::SurfaceKHR{vk_surface};
}

void VulkanTestApp::select_physical_device() {
  this->physical_device = jar::device::pickPhysicalDevice(instance, surface, device_extensions);
}

void VulkanTestApp::create_logical_device() {
  queueFamilyIndices = jar::device::find_queue_families(physical_device, surface);
  float queuePriority = 1.0f;
  vk::DeviceQueueCreateInfo queueCreateInfo{{}, static_cast<uint32_t>(queueFamilyIndices.graphics_family), 1, &queuePriority};

  vk::PhysicalDeviceFeatures deviceFeatures{};

  vk::DeviceCreateInfo createInfo{};
  createInfo.setPQueueCreateInfos(&queueCreateInfo);
  createInfo.setQueueCreateInfoCount(1);
  createInfo.setPEnabledFeatures(&deviceFeatures);
  createInfo.setEnabledExtensionCount(device_extensions.size());
  createInfo.setPpEnabledExtensionNames(device_extensions.data());

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<int> uniqueQueueFamilies = {queueFamilyIndices.graphics_family, queueFamilyIndices.present_family};
  for (int queueFamily : uniqueQueueFamilies) {
    queueCreateInfos.push_back(vk::DeviceQueueCreateInfo{{}, static_cast<uint32_t>(queueFamily), 1, &queuePriority});
  }
  createInfo.setQueueCreateInfoCount(queueCreateInfos.size());
  createInfo.setPQueueCreateInfos(queueCreateInfos.data());


  if (enableValidationLayers) {
    createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
    createInfo.setPpEnabledLayerNames(validationLayers.data());
  } else {
    createInfo.setEnabledLayerCount(0);
  }

  const auto& result = physical_device.createDevice(&createInfo, nullptr, &device);
  if(result != vk::Result::eSuccess) {
    throw std::runtime_error("Failed to create logical device: " + vk::to_string(result)); 
  }

  // And queue
  device.getQueue(queueFamilyIndices.graphics_family, 0, &graphicsQueue);
}

void VulkanTestApp::create_semaphores() {
  vk::SemaphoreCreateInfo semaphoreCreateInfo{};
  for(auto i = 0; i < NUM_FRAME_DATA; i++) {
    aquire_semaphores[i] = device.createSemaphore(semaphoreCreateInfo);
    render_complete_semaphores[i] = device.createSemaphore(semaphoreCreateInfo);
  }
}

void VulkanTestApp::create_graphics_pipeline() {
  auto vert_shader_code = jar::shader::readFile("shaders/vert.spv");
  auto frag_shader_code = jar::shader::readFile("shaders/frag.spv");

  vk::ShaderModule vert_shader_module = jar::shader::create_shader_module(vert_shader_code, device);
  vk::ShaderModule frag_shader_module = jar::shader::create_shader_module(frag_shader_code, device);

  vk::PipelineShaderStageCreateInfo vert_shader_stage_create_info{};
  vert_shader_stage_create_info.setStage(vk::ShaderStageFlagBits::eVertex);
  vert_shader_stage_create_info.setModule(vert_shader_module);
  // The function name of the shader entry point
  // Could be used for some fancy shader building?
  vert_shader_stage_create_info.setPName("main");

  vk::PipelineShaderStageCreateInfo frag_shader_stage_create_info{};
  frag_shader_stage_create_info.setStage(vk::ShaderStageFlagBits::eFragment);
  frag_shader_stage_create_info.setModule(frag_shader_module);
  frag_shader_stage_create_info.setPName("main");

  vk::PipelineShaderStageCreateInfo shaderStages[] = {frag_shader_stage_create_info, vert_shader_stage_create_info};

  device.destroyShaderModule(vert_shader_module);
  device.destroyShaderModule(frag_shader_module);
}

void VulkanTestApp::create_command_pool() {
  vk::CommandPoolCreateInfo commandPoolCreateInfo{};
  // This allows the command buffer to be implicitly reset when vkBeginCommandBuffer is called.
  // You can also explicitly call vkResetCommandBuffer.
  commandPoolCreateInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
  commandPoolCreateInfo.setQueueFamilyIndex(queueFamilyIndices.graphics_family);
  device.createCommandPool(&commandPoolCreateInfo, nullptr, &command_pool);
}

void VulkanTestApp::create_command_buffer() {
  vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
  commandBufferAllocateInfo.setCommandPool(command_pool);
  commandBufferAllocateInfo.setCommandBufferCount(NUM_FRAME_DATA);
  device.allocateCommandBuffers(&commandBufferAllocateInfo, &command_buffer);
  vk::FenceCreateInfo fenceCreateInfo{};

  for(auto i = 0; i < NUM_FRAME_DATA; i++) {
    device.createFence(&fenceCreateInfo, nullptr, &command_buffer_fences[i]);
  }
}

void VulkanTestApp::create_swapchain() {
  SwapChainSupportDetails details = jar::swapchain::query_swapchain_support(physical_device, surface);

  vk::SurfaceFormatKHR surface_format = jar::swapchain::choose_swap_surface_format(details.formats);
  vk::PresentModeKHR present_mode = jar::swapchain::choose_swap_present_mode(details.present_modes);
  vk::Extent2D extent = jar::swapchain::choose_swap_extent(details.capabilities, width, height);

  uint32_t image_count = details.capabilities.minImageCount + 1;
  if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount) {
    image_count = details.capabilities.maxImageCount;
  }

  // Let's create the swapchain, woo!
  vk::SwapchainCreateInfoKHR create_info{};
  create_info.setSurface(surface);
  create_info.setMinImageCount(image_count);
  create_info.setImageFormat(surface_format.format);
  create_info.setImageColorSpace(surface_format.colorSpace);
  create_info.setImageExtent(extent);
  create_info.setImageArrayLayers(1);
  create_info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

  QueueFamilyIndices indices = jar::device::find_queue_families(physical_device, surface);

  if (indices.graphics_family != indices.present_family) {
    //TODO: Understand ownership and image sharing
    create_info.setImageSharingMode(vk::SharingMode::eConcurrent);
    create_info.setQueueFamilyIndexCount(2);
    uint32_t queue_family_indices[] = {(uint32_t) indices.graphics_family, (uint32_t) indices.present_family};
    create_info.setPQueueFamilyIndices(queue_family_indices);
  } else {
    create_info.setImageSharingMode(vk::SharingMode::eExclusive);
    create_info.setQueueFamilyIndexCount(0);
    create_info.setPQueueFamilyIndices(nullptr);
  }

  create_info.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity);
  create_info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
  create_info.setPresentMode(present_mode);
  create_info.setClipped(true);
  create_info.setOldSwapchain(nullptr);

  if (device.createSwapchainKHR(&create_info, nullptr, &(this->swapchain)) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create swap chain!");
  }

  device.getSwapchainImagesKHR(swapchain, &image_count, nullptr);
  swapchain_images.resize(image_count);
  device.getSwapchainImagesKHR(swapchain, &image_count, this->swapchain_images.data());

  swapchain_image_format = surface_format.format;
  swapchain_extent = extent;

}

void VulkanTestApp::create_image_views() {
  swapchain_image_views.resize(swapchain_images.size());
  for(size_t i = 0; i < swapchain_images.size(); i++) {
    vk::ImageViewCreateInfo create_info{};
    create_info.setImage(swapchain_images[i]);
    create_info.setViewType(vk::ImageViewType::e2D);
    create_info.setFormat(swapchain_image_format);
    create_info.components.setR(vk::ComponentSwizzle::eIdentity);
    create_info.components.setG(vk::ComponentSwizzle::eIdentity);
    create_info.components.setB(vk::ComponentSwizzle::eIdentity);
    create_info.components.setA(vk::ComponentSwizzle::eIdentity);

    create_info.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
    create_info.subresourceRange.setBaseMipLevel(0);
    create_info.subresourceRange.setLevelCount(1);
    create_info.subresourceRange.setBaseArrayLayer(0);
    create_info.subresourceRange.setLayerCount(1);

    if(device.createImageView(&create_info, nullptr, &swapchain_image_views[i]) != vk::Result::eSuccess) {
      throw std::runtime_error("failed to create image views!");
    }
  }
}

void VulkanTestApp::mainLoop() {
}

void VulkanTestApp::initVulkan(GLFWwindow* window) {
  this->window = window;
  glfwGetWindowSize(window, &this->width, &this->height);
  if (enableValidationLayers && !jar::validation::checkValidationLayerSupport(validationLayers)) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  /*
  // input and sound systems need to be tied to the new window
  Sys_InitInput();

  // Create the instance
  CreateInstance();
  // Create presentation surface
  CreateSurface();
  // Enumerate physical devices and get their properties
  EnumeratePhysicalDevices();
  // Find queue family/families supporting graphics and present.
  SelectPhysicalDevice();
  // Create logical device and queues
  CreateLogicalDeviceAndQueues();
  // Create semaphores for image acquisition and rendering completion
  CreateSemaphores();
  // Create Command Pool
  CreateCommandPool();
  // Create Command Buffer
  CreateCommandBuffer();
  // Setup the allocator
  vulkanAllocator.Init();
  // Start the Staging Manager
  stagingManager.Init();
  // Create Swap Chain
  CreateSwapChain();
  // Create Render Targets
  CreateRenderTargets();
  // Create Render Pass
  CreateRenderPass();
  // Create Pipeline Cache
  CreatePipelineCache();
  // Create Frame Buffers
  CreateFrameBuffers();
  // Init RenderProg Manager
  renderProgManager.Init();
  */

  create_instance();
  create_surface();
  select_physical_device();
  create_logical_device();
  create_semaphores();
  create_command_pool();
  create_command_buffer();
  create_swapchain();
  create_image_views();
  create_graphics_pipeline();
}

//TODO: RAII this
void VulkanTestApp::cleanup() {
  for(auto i = 0; i < NUM_FRAME_DATA; i++) {
    device.destroySemaphore(aquire_semaphores[i]);
    device.destroySemaphore(render_complete_semaphores[i]);
    device.destroyFence(command_buffer_fences[i]);
  }
  device.destroyCommandPool(command_pool);
  device.destroySwapchainKHR(swapchain);

  for (auto& image_view: swapchain_image_views) {
    device.destroyImageView(image_view, nullptr);
  }

  jar::validation::DestroyDebugReportCallbackEXT(instance, callback, nullptr);
  device.destroy();
  instance.destroySurfaceKHR(surface);
  instance.destroy();
}

void VulkanTestApp::setupDebugCallback() {
  VkDebugReportCallbackCreateInfoEXT createInfo = static_cast<VkDebugReportCallbackCreateInfoEXT>(vk::DebugReportCallbackCreateInfoEXT{
    vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError,
    jar::validation::debugCallback
  });

  if (jar::validation::CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug callback!");
  }
}

#include "VulkanTestApp.hpp"
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include "Validation.hpp"
#include "Device.hpp"
#include "Queue.hpp"

void VulkanTestApp::createLogicalDevice(vk::PhysicalDevice& physicalDevice) {
  queueFamilyIndices = findQueueFamilies(physicalDevice, surface);
  float queuePriority = 1.0f;
  vk::DeviceQueueCreateInfo queueCreateInfo{{}, queueFamilyIndices.graphics_family, 1, &queuePriority};

  vk::PhysicalDeviceFeatures deviceFeatures{};

  vk::DeviceCreateInfo createInfo{};
  createInfo.setPQueueCreateInfos(&queueCreateInfo);
  createInfo.setQueueCreateInfoCount(1);
  createInfo.setPEnabledFeatures(&deviceFeatures);
  createInfo.setEnabledExtensionCount(0);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<int> uniqueQueueFamilies = {queueFamilyIndices.graphics_family, queueFamilyIndices.present_family};
  for (int queueFamily : uniqueQueueFamilies) {
    queueCreateInfos.push_back(vk::DeviceQueueCreateInfo{{}, queueFamily, 1, &queuePriority});
  }
  createInfo.setQueueCreateInfoCount(queueCreateInfos.size());
  createInfo.setPQueueCreateInfos(queueCreateInfos.data());


  if (enableValidationLayers) {
    createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
    createInfo.setPpEnabledLayerNames(validationLayers.data());
  } else {
    createInfo.setEnabledLayerCount(0);
  }

  const auto& result = physicalDevice.createDevice(&createInfo, nullptr, &device);
  if(result != vk::Result::eSuccess) {
    throw std::runtime_error("Failed to create logical device: " + vk::to_string(result)); 
  }
}

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
  std::cout << "Required extensions: \n";
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

void VulkanTestApp::enumerate_physical_devices() {
}

void VulkanTestApp::mainLoop() {
}

void VulkanTestApp::initVulkan(GLFWwindow* window) {
  this->window = window;
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
  enumerate_physical_devices();
  vk::PhysicalDevice physicalDevice = pickPhysicalDevice(instance, surface);
  createLogicalDevice(physicalDevice);
  device.getQueue(queueFamilyIndices.graphics_family, 0, &graphicsQueue);
}

void VulkanTestApp::cleanup() {
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

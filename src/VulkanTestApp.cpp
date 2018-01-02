#include "VulkanTestApp.hpp"
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include "Validation.hpp"
#include "Device.hpp"
#include "Queue.hpp"
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
  auto extensions = getRequiredExtensions(enableValidationLayers);
  createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
  createInfo.setPpEnabledExtensionNames(extensions.data());


  if (vk::createInstance(&createInfo, nullptr, &instance) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create instance!");
  }

  if(enableValidationLayers) {
    setupDebugCallback();
  }
}

void VulkanTestApp::createLogicalDevice(vk::PhysicalDevice& physicalDevice) {
  queueFamilyIndex = findQueueFamilies(physicalDevice, surface);
  float queuePriority = 1.0f;
  vk::DeviceQueueCreateInfo queueCreateInfo{{}, queueFamilyIndex.graphics_family, 1, &queuePriority};

  vk::PhysicalDeviceFeatures deviceFeatures{};

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<int> uniqueQueueFamilies = {queueFamilyIndex.graphics_family, queueFamilyIndex.present_family};
  for (int queueFamily : uniqueQueueFamilies) {
    queueCreateInfos.push_back(vk::DeviceQueueCreateInfo{{}, queueFamily, 1, &queuePriority});
  }

  vk::DeviceCreateInfo createInfo{};
  createInfo.setPQueueCreateInfos(&queueCreateInfo);
  createInfo.setQueueCreateInfoCount(1);
  createInfo.setPEnabledFeatures(&deviceFeatures);
  createInfo.setEnabledExtensionCount(0);
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

VkSurfaceKHR VulkanTestApp::createSurface() {
  VkSurfaceKHR vk_surface{};
  if (glfwCreateWindowSurface(instance, window, nullptr, &vk_surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
  return vk_surface;
}

void VulkanTestApp::mainLoop() {
}

void VulkanTestApp::initVulkan(GLFWwindow* window) {
  this->window = window;
  if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  create_instance();
  this->surface = vk::SurfaceKHR{createSurface()};
  vk::PhysicalDevice physicalDevice = pickPhysicalDevice(instance, surface);
  createLogicalDevice(physicalDevice);
  device.getQueue(queueFamilyIndex.graphics_family, 0, &graphicsQueue);
}

void VulkanTestApp::cleanup() {
  DestroyDebugReportCallbackEXT(instance, callback, nullptr);
  device.destroy();
  instance.destroySurfaceKHR(surface);
  instance.destroy();
}

void VulkanTestApp::setupDebugCallback() {
  VkDebugReportCallbackCreateInfoEXT createInfo = static_cast<VkDebugReportCallbackCreateInfoEXT>(vk::DebugReportCallbackCreateInfoEXT{
    vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError,
    debugCallback
  });

  if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug callback!");
  }
}

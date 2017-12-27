#include <vulkan/vulkan.h>
#include "Validation.hpp"
#include "Device.hpp"
#include "VulkanTestApp.hpp"
#include <iostream>
#include <vector>
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
}

void VulkanTestApp::mainLoop() {
}

void VulkanTestApp::initVulkan() {
  if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) {
    throw std::runtime_error("validation layers requested, but not available!");
  }
  create_instance();
  if(enableValidationLayers) {
    setupDebugCallback();
  }
  pickPhysicalDevice(instance);
}
void VulkanTestApp::cleanup() {
  DestroyDebugReportCallbackEXT(instance, callback, nullptr);
  vkDestroyInstance(instance, nullptr);
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

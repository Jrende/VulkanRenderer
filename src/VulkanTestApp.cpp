#include <vulkan/vulkan.h>
#include "VulkanTestApp.hpp"
#include <iostream>
#include <GLFW/glfw3.h>
#include <vector>
#include <cstring>

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT pCallback, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(instance, pCallback, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* layerPrefix,
    const char* msg,
    void* userData) {
  std::cerr << "validation layer: " << msg << std::endl;
  return VK_FALSE;
}

std::vector<const char*> getRequiredExtensions(bool enableValidationLayers) {
  std::vector<const char*> extensions;

  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  for (uint32_t i = 0; i < glfwExtensionCount; i++) {
    extensions.push_back(glfwExtensions[i]);
  }

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  }

  return extensions;
}

bool checkValidationLayerSupport(const std::vector<const char*> validationLayers) {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char* layerName : validationLayers) {
    bool layerFound = false;

    for (const auto& layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;

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
  auto extensions = getRequiredExtensions(enableValidationLayers);
  createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
  createInfo.setPpEnabledExtensionNames(extensions.data());


  if (vk::createInstance(&createInfo, nullptr, &instance) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create instance!");
  }
}

void VulkanTestApp::setupDebugCallback() {
  VkDebugReportCallbackCreateInfoEXT createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
  createInfo.pfnCallback = debugCallback;

  if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug callback!");
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
}

void VulkanTestApp::cleanup() {
  DestroyDebugReportCallbackEXT(instance, callback, nullptr);
  vkDestroyInstance(instance, nullptr);
}

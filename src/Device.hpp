#include <vulkan/vulkan.hpp>
#include <vector>
#include "QueueFamilyIndices.hpp"
QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface) {
  uint32_t queueFamilyCount = 0;
  physicalDevice.getQueueFamilyProperties(&queueFamilyCount, nullptr);

  std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
  physicalDevice.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());

  uint32_t i = 0;
  QueueFamilyIndices indices{};
  for (const auto& queueFamily : queueFamilies) {
    vk::Bool32 presentSupport = false;
    physicalDevice.getSurfaceSupportKHR(i, surface, &presentSupport);
    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.present_family = i;
    }

    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
      indices.graphics_family = i;
    }
    if(indices.is_complete()) {
      break;
    }
    i++;
  }
  return indices;
}

//Is the user has a graphics card, that's probably enough
bool isDeviceSuitable(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) {
  const auto& foundIndex = findQueueFamilies(device, surface);
  return foundIndex.is_complete();
}

vk::PhysicalDevice pickPhysicalDevice(vk::Instance& instance, const vk::SurfaceKHR& surface) {
  vk::PhysicalDevice physicalDevice{};
  uint32_t deviceCount = 0;
  instance.enumeratePhysicalDevices(&deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  std::vector<vk::PhysicalDevice> devices(deviceCount);
  instance.enumeratePhysicalDevices(&deviceCount, devices.data());

  bool found = false;
  std::cout << "Physical devices: \n";
  for(const auto& device: devices) {
    vk::PhysicalDeviceFeatures deviceFeatures{};
    device.getFeatures(&deviceFeatures);
    vk::PhysicalDeviceProperties deviceProperties{};
    device.getProperties(&deviceProperties);
    if (isDeviceSuitable(device, surface)) {
      std::cout << "Using " << deviceProperties.deviceName << '\n';
      found = true;
      physicalDevice = device;
      break;
    }
  }

  if (!found) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  return physicalDevice;
}


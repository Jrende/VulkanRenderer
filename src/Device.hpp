#include <vulkan/vulkan.hpp>
#include <vector>
#include "QueueFamilyIndices.hpp"

std::vector<vk::QueueFamilyProperties> getQueueFamilies(const vk::PhysicalDevice& physicalDevice) {
  uint32_t queueFamilyCount = 0;
  physicalDevice.getQueueFamilyProperties(&queueFamilyCount, nullptr);

  std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
  physicalDevice.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());
  return queueFamilies;
}

bool isDeviceSuitable(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface) {
  uint32_t i = 0;
  const auto queueFamilies = getQueueFamilies(physicalDevice);
  for (const auto& queueFamily : queueFamilies) {
    vk::Bool32 presentSupport = false;
    physicalDevice.getSurfaceSupportKHR(i, surface, &presentSupport);
    if(queueFamily.queueCount > 0 &&
        presentSupport &&
        (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)) {
      return true;
    }
  }
  return false;
}

const std::vector<vk::PhysicalDevice> get_devices(vk::Instance& instance) {
  uint32_t deviceCount = 0;
  instance.enumeratePhysicalDevices(&deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  std::vector<vk::PhysicalDevice> devices(deviceCount);
  instance.enumeratePhysicalDevices(&deviceCount, devices.data());
  return devices;
}

vk::PhysicalDevice pickPhysicalDevice(vk::Instance& instance, const vk::SurfaceKHR& surface) {
  vk::PhysicalDevice physicalDevice{};

  bool found = false;
  const auto devices = get_devices(instance);
  std::cout << "Found " << devices.size() << " device(s)\n";
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


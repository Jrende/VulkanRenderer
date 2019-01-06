#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <set>
#include "QueueFamilyIndices.hpp"
#include "SwapChainSupportDetails.hpp"
#include "SwapChain.hpp"

namespace jar::device {

  std::vector<vk::QueueFamilyProperties> getQueueFamilies(const vk::PhysicalDevice& physicalDevice) {
    uint32_t queueFamilyCount = 0;
    physicalDevice.getQueueFamilyProperties(&queueFamilyCount, nullptr);

    std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
    physicalDevice.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());
    return queueFamilies;
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

  QueueFamilyIndices find_queue_families(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface) {
    uint32_t i = 0;
    const std::vector<vk::QueueFamilyProperties>& queueFamilies = getQueueFamilies(physicalDevice);
    QueueFamilyIndices indices{};
    for (const auto& queueFamily : queueFamilies) {
      vk::Bool32 presentSupport = false;
      physicalDevice.getSurfaceSupportKHR(i, surface, &presentSupport);
      if(queueFamily.queueCount > 0) {
        if(presentSupport) {
          indices.present_family = i;
        }

        if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
          indices.graphics_family = i;
        }
      }
      if(indices.is_complete()) {
        break;
      }
      i++;
    }
    return indices;
  }

  bool check_device_extension_support(const vk::PhysicalDevice& device, const std::vector<const char*> device_extensions) {
    uint32_t extension_count;
    device.enumerateDeviceExtensionProperties(nullptr, &extension_count, nullptr);

    std::vector<vk::ExtensionProperties> available_extensions(extension_count);
    device.enumerateDeviceExtensionProperties(nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

    for (const auto& extension: available_extensions) {
      required_extensions.erase(extension.extensionName);
    }

    if(!required_extensions.empty()) {
      vk::PhysicalDeviceProperties deviceProperties{};
      device.getProperties(&deviceProperties);
      std::cout << "Device " << deviceProperties.deviceName << " is missing the following device extensions to be considered suitable:\n";
      for(const auto& ext: required_extensions) {
        std::cout << '\t' << ext << '\n';
      }
    }

    return required_extensions.empty(); 
  }

  bool isDeviceSuitable(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface, const std::vector<const char*> required_device_extensions) {
    QueueFamilyIndices foundIndex = find_queue_families(device, surface);
    bool extensions_supported = check_device_extension_support(device, required_device_extensions);

    bool swapchain_adequate = false;
    if (extensions_supported) {
      SwapChainSupportDetails swapchain_support = jar::swapchain::query_swapchain_support(device, surface);
      swapchain_adequate = !swapchain_support.formats.empty() && !swapchain_support.present_modes.empty();
    }
    return foundIndex.is_complete() && extensions_supported && swapchain_adequate;
  }


  vk::PhysicalDevice pickPhysicalDevice(vk::Instance& instance,
      const vk::SurfaceKHR& surface,
      const std::vector<const char*> required_device_extensions) {
    vk::PhysicalDevice physicalDevice{};

    bool found = false;
    const auto devices = get_devices(instance);
    std::cout << "Found " << devices.size() << " device(s)\n";
    for(const auto& device: devices) {
      vk::PhysicalDeviceFeatures deviceFeatures{};
      device.getFeatures(&deviceFeatures);
      vk::PhysicalDeviceProperties deviceProperties{};
      device.getProperties(&deviceProperties);

      if (isDeviceSuitable(device, surface, required_device_extensions)) {
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
}

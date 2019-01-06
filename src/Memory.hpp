#pragma once
#include <vulkan/vulkan.hpp>
namespace jar::memory {
  uint32_t findMemoryType(const vk::PhysicalDevice& physical_device, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties mem_properties;
    physical_device.getMemoryProperties(&mem_properties);

    for(size_t i = 0; i < mem_properties.memoryTypeCount; i++) {
      const auto& memory_type = mem_properties.memoryTypes[i];
      if((typeFilter & (1 << i)) && (properties & memory_type.propertyFlags) == properties) {
        return i;
      }
    }


    throw std::runtime_error("failed to find suitable memory type!");
  }
}

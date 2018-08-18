#include <set>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "SwapChainSupportDetails.hpp"

#pragma once

namespace jar::swapchain {

  SwapChainSupportDetails query_swapchain_support(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) {
    SwapChainSupportDetails details{};
    device.getSurfaceCapabilitiesKHR(surface, &details.capabilities);

    uint32_t format_count;
    device.getSurfaceFormatsKHR(surface, &format_count, nullptr);

    if (format_count != 0) {
      details.formats.resize(format_count);
      device.getSurfaceFormatsKHR(surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    device.getSurfacePresentModesKHR(surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
      details.present_modes.resize(present_mode_count);
      device.getSurfacePresentModesKHR(surface, &present_mode_count, details.present_modes.data());
    }

    return details;
  }

  vk::SurfaceFormatKHR choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats) {
    if (available_formats.size() == 1 && available_formats[0].format == vk::Format::eUndefined) {
      return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
    }
    for (const auto& available_format : available_formats) {
      if (available_format.format == vk::Format::eB8G8R8A8Unorm && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
        return available_format;
      }
    }

    return available_formats[0];
  }

  vk::PresentModeKHR choose_swap_present_mode(const std::vector<vk::PresentModeKHR> available_present_modes) {
    for (const auto& available_present_modes : available_present_modes) {
        if (available_present_modes == vk::PresentModeKHR::eMailbox) {
            return available_present_modes;
        }
    }

    return vk::PresentModeKHR::eFifo;
  }

  vk::Extent2D choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
    } else {
      vk::Extent2D actual_extent = {width, height};

      actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
      actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

      return actual_extent;
    }
  }
}

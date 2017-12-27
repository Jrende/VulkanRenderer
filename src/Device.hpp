#include <vulkan/vulkan.hpp>

bool isDeviceSuitable(const vk::PhysicalDevice& device) {
    return true;
}

vk::PhysicalDevice pickPhysicalDevice(vk::Instance& instance) {
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
    vk::PhysicalDeviceProperties deviceProperties{};
    device.getProperties(&deviceProperties);
    std::cout << deviceProperties.deviceName << '\n';
    if (isDeviceSuitable(device)) {
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


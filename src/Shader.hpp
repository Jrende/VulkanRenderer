#include <fstream>
#pragma once

namespace jar::shader {
  static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
  }

  vk::ShaderModule create_shader_module(const std::vector<char>& source, const vk::Device& device) {
    vk::ShaderModuleCreateInfo create_info{};
    create_info.setCodeSize(source.size());
    create_info.setPCode(reinterpret_cast<const uint32_t*>(source.data()));
    vk::ShaderModule shader_module;
    if(device.createShaderModule(&create_info, nullptr, &shader_module) != vk::Result::eSuccess) {
      throw std::runtime_error("failed to create shader module!");
    }
    return shader_module;
  }
}

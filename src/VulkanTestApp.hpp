#include <vulkan/vulkan.hpp>
#include <string>

class VulkanTestApp {
  private:
  vk::Instance instance;
  VkDebugReportCallbackEXT callback;
  const bool enableValidationLayers = true;
  const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
  };
  void create_instance();
  void setupDebugCallback();

  public:
    void mainLoop();
    void initVulkan();
    void cleanup();
};

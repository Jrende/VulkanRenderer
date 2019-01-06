#include "VulkanTestApp.hpp"
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include "Validation.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"
#include "SwapChainSupportDetails.hpp"
#include "Shader.hpp"
#include "Memory.hpp"

void VulkanTestApp::create_instance() {
  vk::ApplicationInfo appInfo{
    "Hello Triangle",
    VK_MAKE_VERSION(1, 0, 0),
    "Jar",
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
  auto extensions = jar::validation::get_required_extensions(enableValidationLayers);
  std::cout << "Required instance extensions: \n";
  for(const auto& e: extensions) {
    std::cout << "\t" << e << '\n';
  }
  
  createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
  createInfo.setPpEnabledExtensionNames(extensions.data());

  if(enableValidationLayers) {
    createInfo.setEnabledLayerCount(validationLayers.size());
    createInfo.setPpEnabledLayerNames(validationLayers.data());
  }

  if (vk::createInstance(&createInfo, nullptr, &(this->instance)) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create instance!");
  }

  if(enableValidationLayers) {
    setupDebugCallback();
  }
}

void VulkanTestApp::create_surface() {
  VkSurfaceKHR vk_surface{};
  if (glfwCreateWindowSurface(instance, window, nullptr, &vk_surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
  this->surface = vk::SurfaceKHR{vk_surface};
}

void VulkanTestApp::select_physical_device() {
  this->physical_device = jar::device::pickPhysicalDevice(instance, surface, device_extensions);
}

void VulkanTestApp::create_logical_device() {
  queueFamilyIndices = jar::device::find_queue_families(physical_device, surface);
  float queuePriority = 1.0f;
  vk::DeviceQueueCreateInfo queueCreateInfo{{}, static_cast<uint32_t>(queueFamilyIndices.graphics_family), 1, &queuePriority};

  vk::PhysicalDeviceFeatures deviceFeatures{};

  vk::DeviceCreateInfo createInfo{};
  createInfo.setPQueueCreateInfos(&queueCreateInfo);
  createInfo.setQueueCreateInfoCount(1);
  createInfo.setPEnabledFeatures(&deviceFeatures);
  createInfo.setEnabledExtensionCount(device_extensions.size());
  createInfo.setPpEnabledExtensionNames(device_extensions.data());

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<int> uniqueQueueFamilies = {queueFamilyIndices.graphics_family, queueFamilyIndices.present_family};
  for (int queue_family : uniqueQueueFamilies) {
    vk::DeviceQueueCreateInfo queue_create_info{};
    queue_create_info.setQueueFamilyIndex(queue_family);
    queue_create_info.setQueueCount(1);
    queue_create_info.setPQueuePriorities(&queuePriority);
    queueCreateInfos.push_back(queue_create_info);
  }
  createInfo.setQueueCreateInfoCount(queueCreateInfos.size());
  createInfo.setPQueueCreateInfos(queueCreateInfos.data());


  if (enableValidationLayers) {
    createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
    createInfo.setPpEnabledLayerNames(validationLayers.data());
  } else {
    createInfo.setEnabledLayerCount(0);
  }

  const auto& result = physical_device.createDevice(&createInfo, nullptr, &device);
  if(result != vk::Result::eSuccess) {
    throw std::runtime_error("Failed to create logical device: " + vk::to_string(result)); 
  }

  // And queue
  device.getQueue(queueFamilyIndices.graphics_family, 0, &graphics_queue);
  device.getQueue(queueFamilyIndices.present_family, 0, &present_queue);
}

void VulkanTestApp::create_semaphores() {
  image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
  render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
  in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
  for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    auto& image_available_semaphore = image_available_semaphores[i];
    auto& render_finished_semaphore = render_finished_semaphores[i];
    vk::SemaphoreCreateInfo semaphore_info{};
    vk::FenceCreateInfo fence_info{};
    fence_info.setFlags(vk::FenceCreateFlagBits::eSignaled);
    if(
        device.createSemaphore(&semaphore_info, nullptr, &image_available_semaphore) != vk::Result::eSuccess ||
        device.createSemaphore(&semaphore_info, nullptr, &render_finished_semaphore) != vk::Result::eSuccess ||
        device.createFence(&fence_info, nullptr, &in_flight_fences[i]) != vk::Result::eSuccess) {
      throw std::runtime_error("Failed to create semaphores!"); 
    }
  }
}

void VulkanTestApp::create_graphics_pipeline() {
  auto vert_shader_code = jar::shader::readFile("shaders/vert.spv");
  auto frag_shader_code = jar::shader::readFile("shaders/frag.spv");

  vk::ShaderModule vert_shader_module = jar::shader::create_shader_module(vert_shader_code, device);
  vk::ShaderModule frag_shader_module = jar::shader::create_shader_module(frag_shader_code, device);

  vk::PipelineShaderStageCreateInfo vert_shader_stage_create_info{};
  vert_shader_stage_create_info.setStage(vk::ShaderStageFlagBits::eVertex);
  vert_shader_stage_create_info.setModule(vert_shader_module);
  // The function name of the shader entry point
  // Could be used for some fancy shader building?
  vert_shader_stage_create_info.setPName("main");

  vk::PipelineShaderStageCreateInfo frag_shader_stage_create_info{};
  frag_shader_stage_create_info.setStage(vk::ShaderStageFlagBits::eFragment);
  frag_shader_stage_create_info.setModule(frag_shader_module);
  frag_shader_stage_create_info.setPName("main");

  vk::PipelineShaderStageCreateInfo shader_stages[] = {frag_shader_stage_create_info, vert_shader_stage_create_info};

  vk::PipelineVertexInputStateCreateInfo vertex_info{};
  auto bindingDescription = Vertex::getBindingDescription();
  vertex_info.setVertexBindingDescriptionCount(1);
  vertex_info.setPVertexBindingDescriptions(&bindingDescription);
  auto attributeDescription = Vertex::getAttributeDescriptions();
  vertex_info.setVertexAttributeDescriptionCount(attributeDescription.size());
  vertex_info.setPVertexAttributeDescriptions(attributeDescription.data());

  vk::PipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.setTopology(vk::PrimitiveTopology::eTriangleList);
  input_assembly.setPrimitiveRestartEnable(false);

  vk::Viewport viewport{};
  viewport.setX(0.0f);
  viewport.setY(0.0f);
  viewport.setWidth((float) swapchain_extent.width);
  viewport.setHeight((float) swapchain_extent.height);
  viewport.setMinDepth(0.0f);
  viewport.setMaxDepth(1.0f);

  vk::Rect2D scissor = {};
  scissor.setOffset({0, 0});
  scissor.setExtent(swapchain_extent);

  vk::PipelineViewportStateCreateInfo viewport_state = {};
  viewport_state.setViewportCount(1);
  viewport_state.setPViewports(&viewport);
  viewport_state.setScissorCount(1);
  viewport_state.setPScissors(&scissor);

  vk::PipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.setDepthClampEnable(false);
  rasterizer.setRasterizerDiscardEnable(false);
  rasterizer.setPolygonMode(vk::PolygonMode::eFill);
  rasterizer.setLineWidth(1.0f);
  rasterizer.setCullMode(vk::CullModeFlagBits::eBack);
  rasterizer.setFrontFace(vk::FrontFace::eClockwise);
  //Changing these might be useful for shadow mapping
  rasterizer.setDepthBiasEnable(false);
  rasterizer.setDepthBiasConstantFactor(0.0f);
  rasterizer.setDepthBiasClamp(0.0f);
  rasterizer.setDepthBiasSlopeFactor(0.0f);

  vk::PipelineMultisampleStateCreateInfo multisampling{};
  multisampling.setSampleShadingEnable(false);
  multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
  multisampling.setMinSampleShading(1.0f);
  multisampling.setPSampleMask(nullptr);
  multisampling.setAlphaToCoverageEnable(false);
  multisampling.setAlphaToOneEnable(false);

  vk::PipelineColorBlendAttachmentState color_blend_attachment{};
  color_blend_attachment.setColorWriteMask(vk::ColorComponentFlagBits::eR |vk::ColorComponentFlagBits::eG |vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
  color_blend_attachment.setBlendEnable(false);
  color_blend_attachment.setSrcColorBlendFactor(vk::BlendFactor::eOne);
  color_blend_attachment.setDstColorBlendFactor(vk::BlendFactor::eZero);
  color_blend_attachment.setColorBlendOp(vk::BlendOp::eAdd);
  color_blend_attachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
  color_blend_attachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
  color_blend_attachment.setAlphaBlendOp(vk::BlendOp::eAdd);

  vk::PipelineColorBlendStateCreateInfo color_blending;
  color_blending.setLogicOpEnable(false);
  color_blending.setLogicOp(vk::LogicOp::eCopy);
  color_blending.setAttachmentCount(1);
  color_blending.setPAttachments(&color_blend_attachment);
  color_blending.setBlendConstants({0.0, 0.0, 0.0, 0.0});

  vk::DynamicState dynamic_states[] = {
    vk::DynamicState::eViewport,
    vk::DynamicState::eLineWidth
  };

  vk::PipelineDynamicStateCreateInfo dynamic_state{};
  dynamic_state.setDynamicStateCount(2);
  dynamic_state.setPDynamicStates(dynamic_states);

  vk::PipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.setSetLayoutCount(0);
  pipeline_layout_info.setPSetLayouts(nullptr);
  pipeline_layout_info.setPushConstantRangeCount(0);
  pipeline_layout_info.setPPushConstantRanges(nullptr);

  if (device.createPipelineLayout(&pipeline_layout_info, nullptr, &pipeline_layout) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  vk::GraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.setStageCount(2);
  pipeline_info.setPStages(shader_stages);
  pipeline_info.setPVertexInputState(&vertex_info);
  pipeline_info.setPInputAssemblyState(&input_assembly);
  pipeline_info.setPViewportState(&viewport_state);
  pipeline_info.setPRasterizationState(&rasterizer);
  pipeline_info.setPMultisampleState(&multisampling);
  pipeline_info.setPDepthStencilState(nullptr); // Optional
  pipeline_info.setPColorBlendState(&color_blending);
  pipeline_info.setPDynamicState(nullptr); // Optional
  pipeline_info.setLayout(pipeline_layout);
  pipeline_info.setRenderPass(render_pass);
  pipeline_info.setSubpass(0);
  pipeline_info.setBasePipelineHandle(nullptr);
  pipeline_info.setBasePipelineIndex(-1);

  if(device.createGraphicsPipelines(nullptr, 1, &pipeline_info, nullptr, &graphics_pipeline) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }
  device.destroyShaderModule(vert_shader_module);
  device.destroyShaderModule(frag_shader_module);
}


void VulkanTestApp::create_model_buffer() {
  vk::DeviceSize vertex_size = sizeof(vertices[0]) * vertices.size();
  vk::DeviceSize index_size = sizeof(indices[0]) * indices.size();
  vk::DeviceSize buffer_size = vertex_size + index_size;

  vk::Buffer staging_buffer;
  vk::DeviceMemory staging_buffer_memory;
  create_buffer(buffer_size,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
      staging_buffer,
      staging_buffer_memory);

  void* data = device.mapMemory(staging_buffer_memory, 0, buffer_size);
  memcpy(static_cast<char*>(data), vertices.data(), vertex_size);
  memcpy(static_cast<char*>(data) + vertex_size, indices.data(), index_size);
  device.unmapMemory(staging_buffer_memory);

  create_buffer(buffer_size,
      vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
      vk::MemoryPropertyFlagBits::eDeviceLocal,
      model_buffer,
      model_buffer_memory);

  copy_buffer(staging_buffer, model_buffer, buffer_size);
  device.destroyBuffer(staging_buffer, nullptr);
  device.freeMemory(staging_buffer_memory, nullptr);
}

/*
void VulkanTestApp::create_index_buffer() {
  vk::DeviceSize buffer_size = sizeof(indices[0]) * indices.size();

  vk::Buffer staging_buffer;
  vk::DeviceMemory staging_buffer_memory;
  create_buffer(buffer_size,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
      staging_buffer,
      staging_buffer_memory);

  void* data = device.mapMemory(staging_buffer_memory, 0, buffer_size);
  memcpy(data, indices.data(), (size_t) buffer_size);
  device.unmapMemory(staging_buffer_memory);

  create_buffer(buffer_size,
      vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
      vk::MemoryPropertyFlagBits::eDeviceLocal,
      index_buffer,
      index_buffer_memory);

  copy_buffer(staging_buffer, index_buffer, buffer_size);
  device.destroyBuffer(staging_buffer, nullptr);
  device.freeMemory(staging_buffer_memory, nullptr);
}
*/

void VulkanTestApp::create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& buffer_memory) {
  vk::BufferCreateInfo buffer_info{};
  buffer_info.setSize(size);
  buffer_info.setUsage(usage);
  buffer_info.setSharingMode(vk::SharingMode::eExclusive);

  if (device.createBuffer(&buffer_info, nullptr, &buffer) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  vk::MemoryRequirements mem_requirements = device.getBufferMemoryRequirements(buffer);


  const auto& memory_type_index = jar::memory::findMemoryType(physical_device,
      mem_requirements.memoryTypeBits,
      properties);
  vk::MemoryAllocateInfo allocInfo = {};
  allocInfo.setAllocationSize(mem_requirements.size);
  allocInfo.setMemoryTypeIndex(memory_type_index);

  if (device.allocateMemory(&allocInfo, nullptr, &buffer_memory) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }

  device.bindBufferMemory(buffer, buffer_memory, 0);
}

void VulkanTestApp::copy_buffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size) {
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandPool(command_pool);
    allocInfo.setCommandBufferCount(1);

    vk::CommandBuffer command_buffer;
    device.allocateCommandBuffers(&allocInfo, &command_buffer);

    vk::CommandBufferBeginInfo begin_info = {};
    begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    command_buffer.begin(&begin_info);

    vk::BufferCopy copy_region = {};
    copy_region.setSize(size);
    command_buffer.copyBuffer(src_buffer, dst_buffer, 1, &copy_region);

    command_buffer.end();
    vk::SubmitInfo submit_info = {};
    submit_info.setCommandBufferCount(1);
    submit_info.setPCommandBuffers(&command_buffer);

    graphics_queue.submit(1, &submit_info, nullptr);
    graphics_queue.waitIdle();
    device.freeCommandBuffers(command_pool, 1, &command_buffer);
}

void VulkanTestApp::create_command_buffers() {
  command_buffers.resize(swapchain_framebuffers.size());
  vk::CommandBufferAllocateInfo alloc_info{};
  alloc_info.setCommandPool(command_pool);
  alloc_info.setLevel(vk::CommandBufferLevel::ePrimary);
  alloc_info.setCommandBufferCount((uint32_t) command_buffers.size());

  if(device.allocateCommandBuffers(&alloc_info, command_buffers.data()) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create command buffers!");
  }
  vk::FenceCreateInfo fence_create_info{};

  for(size_t i = 0; i < command_buffers.size(); i++) {
    vk::CommandBufferBeginInfo begin_info{};
    auto& cmd_buf = command_buffers[i];
    begin_info.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
    begin_info.setPInheritanceInfo(nullptr);
    if(cmd_buf.begin(&begin_info) != vk::Result::eSuccess) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }
    vk::RenderPassBeginInfo render_pass_info{};
    render_pass_info.setRenderPass(render_pass);
    render_pass_info.setFramebuffer(swapchain_framebuffers[i]);
    render_pass_info.renderArea.setOffset({0, 0});
    render_pass_info.renderArea.setExtent(swapchain_extent);
    vk::ClearValue clear_color{std::array<float, 4>{0.5, 0.7f, 0.2f, 1.0f}};
    render_pass_info.setClearValueCount(1);
    render_pass_info.setPClearValues(&clear_color);
    cmd_buf.beginRenderPass(&render_pass_info, vk::SubpassContents::eInline);


    cmd_buf.bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipeline);
    vk::Buffer model_buffers[] = {model_buffer};
    vk::DeviceSize offsets[] = {0};
    cmd_buf.bindVertexBuffers(0, 1, model_buffers, offsets);

    vk::DeviceSize vertex_size = sizeof(vertices[0]) * vertices.size();
    cmd_buf.bindIndexBuffer(model_buffer, vertex_size, vk::IndexType::eUint16);
    cmd_buf.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    cmd_buf.endRenderPass();
    cmd_buf.end();
  }
}

void VulkanTestApp::create_swapchain() {
  SwapChainSupportDetails details = jar::swapchain::query_swapchain_support(physical_device, surface);

  vk::SurfaceFormatKHR surface_format = jar::swapchain::choose_swap_surface_format(details.formats);
  vk::PresentModeKHR present_mode = jar::swapchain::choose_swap_present_mode(details.present_modes);
  vk::Extent2D extent = jar::swapchain::choose_swap_extent(details.capabilities, width, height);

  uint32_t image_count = details.capabilities.minImageCount + 1;
  if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount) {
    image_count = details.capabilities.maxImageCount;
  }

  // Let's create the swapchain, woo!
  vk::SwapchainCreateInfoKHR create_info{};
  create_info.setSurface(surface);
  create_info.setMinImageCount(image_count);
  create_info.setImageFormat(surface_format.format);
  create_info.setImageColorSpace(surface_format.colorSpace);
  create_info.setImageExtent(extent);
  create_info.setImageArrayLayers(1);
  create_info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

  QueueFamilyIndices indices = jar::device::find_queue_families(physical_device, surface);

  if (indices.graphics_family != indices.present_family) {
    //TODO: Understand ownership and image sharing
    create_info.setImageSharingMode(vk::SharingMode::eConcurrent);
    create_info.setQueueFamilyIndexCount(2);
    uint32_t queue_family_indices[] = {(uint32_t) indices.graphics_family, (uint32_t) indices.present_family};
    create_info.setPQueueFamilyIndices(queue_family_indices);
  } else {
    create_info.setImageSharingMode(vk::SharingMode::eExclusive);
    create_info.setQueueFamilyIndexCount(0);
    create_info.setPQueueFamilyIndices(nullptr);
  }

  create_info.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity);
  create_info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
  create_info.setPresentMode(present_mode);
  create_info.setClipped(true);
  create_info.setOldSwapchain(nullptr);

  if (device.createSwapchainKHR(&create_info, nullptr, &(this->swapchain)) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create swap chain!");
  }

  device.getSwapchainImagesKHR(swapchain, &image_count, nullptr);
  swapchain_images.resize(image_count);
  device.getSwapchainImagesKHR(swapchain, &image_count, this->swapchain_images.data());

  swapchain_image_format = surface_format.format;
  swapchain_extent = extent;

}

void VulkanTestApp::create_image_views() {
  swapchain_image_views.resize(swapchain_images.size());
  for(size_t i = 0; i < swapchain_images.size(); i++) {
    vk::ImageViewCreateInfo create_info{};
    create_info.setImage(swapchain_images[i]);
    create_info.setViewType(vk::ImageViewType::e2D);
    create_info.setFormat(swapchain_image_format);
    create_info.components.setR(vk::ComponentSwizzle::eIdentity);
    create_info.components.setG(vk::ComponentSwizzle::eIdentity);
    create_info.components.setB(vk::ComponentSwizzle::eIdentity);
    create_info.components.setA(vk::ComponentSwizzle::eIdentity);

    create_info.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
    create_info.subresourceRange.setBaseMipLevel(0);
    create_info.subresourceRange.setLevelCount(1);
    create_info.subresourceRange.setBaseArrayLayer(0);
    create_info.subresourceRange.setLayerCount(1);

    if(device.createImageView(&create_info, nullptr, &swapchain_image_views[i]) != vk::Result::eSuccess) {
      throw std::runtime_error("failed to create image views!");
    }
  }
}

void VulkanTestApp::create_render_pass() {
  vk::AttachmentDescription color_attachment{};
  color_attachment.setFormat(swapchain_image_format);
  color_attachment.setSamples(vk::SampleCountFlagBits::e1);
  color_attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
  color_attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
  color_attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
  color_attachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
  color_attachment.setInitialLayout(vk::ImageLayout::eUndefined);
  color_attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

  vk::AttachmentReference color_attachment_ref{};
  color_attachment_ref.setAttachment(0);
  color_attachment_ref.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

  vk::SubpassDescription subpass{};
  subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
  subpass.setColorAttachmentCount(1);
  subpass.setPColorAttachments(&color_attachment_ref);

  vk::RenderPassCreateInfo render_pass_info{};
  render_pass_info.setAttachmentCount(1);
  render_pass_info.setPAttachments(&color_attachment);
  render_pass_info.setSubpassCount(1);
  render_pass_info.setPSubpasses(&subpass);

  // TODO: try to render without this
  vk::SubpassDependency dependency{};
  dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
  dependency.setDstSubpass(0);

  dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
  // ??
  dependency.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead);

  dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
  dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

  render_pass_info.setDependencyCount(1);
  render_pass_info.setPDependencies(&dependency);

  if(device.createRenderPass(&render_pass_info, nullptr, &render_pass) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void VulkanTestApp::create_frame_buffers() {
  swapchain_framebuffers.resize(swapchain_image_views.size());
  for(size_t i = 0; i < swapchain_image_views.size(); i++) {
    vk::ImageView attachments[] = {
      swapchain_image_views[i]
    };

    vk::FramebufferCreateInfo framebuffer_info{};
    framebuffer_info.setRenderPass(render_pass);
    framebuffer_info.setAttachmentCount(1);
    framebuffer_info.setPAttachments(attachments);
    framebuffer_info.setWidth(swapchain_extent.width);
    framebuffer_info.setHeight(swapchain_extent.height);
    framebuffer_info.setLayers(1);

    if (device.createFramebuffer(&framebuffer_info, nullptr, &swapchain_framebuffers[i]) != vk::Result::eSuccess) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void VulkanTestApp::create_command_pool() {
  QueueFamilyIndices queue_family_indices = jar::device::find_queue_families(physical_device, surface);
  vk::CommandPoolCreateInfo pool_info{};
  pool_info.setQueueFamilyIndex(queue_family_indices.graphics_family);
  // not sure if correct flag
  //pool_info.setFlags(vk::CommandPoolCreateFlagBits::eTransient);

  if (device.createCommandPool(&pool_info, nullptr, &command_pool) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create command pool!");
  }
}

void VulkanTestApp::init_vulkan(GLFWwindow* window) {
  this->window = window;
  glfwGetWindowSize(window, &this->width, &this->height);
  if (enableValidationLayers && !jar::validation::checkValidationLayerSupport(validationLayers)) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  create_instance();
  create_surface();
  select_physical_device();
  create_logical_device();
  create_swapchain();
  create_image_views();
  create_render_pass();
  create_graphics_pipeline();
  create_frame_buffers();
  create_command_pool();
  create_model_buffer();
  create_command_buffers();
  create_semaphores();
}

void VulkanTestApp::draw_frame() {
  device.waitForFences(1, &in_flight_fences[current_frame], true, std::numeric_limits<uint64_t>::max());
  device.resetFences(1, &in_flight_fences[current_frame]);

  uint32_t image_index;
  const auto& image_available_semaphore = image_available_semaphores[current_frame];
  const auto& render_finished_semaphore = render_finished_semaphores[current_frame];
  device.acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), image_available_semaphore, nullptr, &image_index);

  vk::SubmitInfo submit_info{};

  vk::Semaphore wait_semaphores[] = {image_available_semaphore};
  vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  submit_info.setWaitSemaphoreCount(1);
  submit_info.setPWaitSemaphores(wait_semaphores);
  submit_info.setPWaitDstStageMask(wait_stages);

  submit_info.setCommandBufferCount(1);
  submit_info.setPCommandBuffers(&command_buffers[image_index]);
  vk::Semaphore signal_semaphores[] = {render_finished_semaphore};
  submit_info.setSignalSemaphoreCount(1);
  submit_info.setPSignalSemaphores(signal_semaphores);
  if (graphics_queue.submit(1, &submit_info, in_flight_fences[current_frame]) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  vk::PresentInfoKHR present_info{};
  present_info.setWaitSemaphoreCount(1);
  present_info.setPWaitSemaphores(signal_semaphores);
  vk::SwapchainKHR swapchains[] = {swapchain};
  present_info.setSwapchainCount(1);
  present_info.setPSwapchains(swapchains);
  present_info.setPImageIndices(&image_index);
  present_info.setPResults(nullptr);
  present_queue.presentKHR(&present_info);
  current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

//TODO: RAII this
void VulkanTestApp::cleanup() {
  std::cout << "Cleanup\n";
  device.waitIdle();
  for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    device.destroySemaphore(render_finished_semaphores[i]);
    device.destroySemaphore(image_available_semaphores[i]);
    device.destroyFence(in_flight_fences[i]);
  }
  for(const auto& framebuffer: swapchain_framebuffers) {
    device.destroyFramebuffer(framebuffer);
  }
  device.destroySwapchainKHR(swapchain);
  device.destroyBuffer(model_buffer);
  device.freeMemory(model_buffer_memory, nullptr);
  device.destroyRenderPass(render_pass);
  device.destroyPipelineLayout(pipeline_layout);
  device.destroyPipeline(graphics_pipeline);
  device.destroyCommandPool(command_pool);


  for (auto& image_view: swapchain_image_views) {
    device.destroyImageView(image_view);
  }

  jar::validation::DestroyDebugReportCallbackEXT(instance, callback, nullptr);
  device.destroy();
  instance.destroySurfaceKHR(surface);
  instance.destroy();
  std::cout << "Everything destroyed\n";
}

void VulkanTestApp::setupDebugCallback() {
  VkDebugReportCallbackCreateInfoEXT createInfo = static_cast<VkDebugReportCallbackCreateInfoEXT>(vk::DebugReportCallbackCreateInfoEXT{
    vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError,
    jar::validation::debugCallback
  });

  if (jar::validation::CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug callback!");
  }
}

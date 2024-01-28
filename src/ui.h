#pragma once

#include "window.h"
#include "device.h"
#include "renderer.h"
#include "descriptor.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

namespace VulkanEngine
{

class UI
{
public:
    UI(Window &window, Device &device, Renderer &renderer, std::unique_ptr<DescriptorPool> &descriptorPool);
    ~UI();
    void render(VkCommandBuffer commandBuffer);
};

}
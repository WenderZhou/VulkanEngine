#pragma once

#include "camera.h"

#include <vulkan/vulkan.h>

namespace VulkanEngine
{
struct FrameInfo
{
	int frameIndex;
	float frameTime;
	VkCommandBuffer commandBuffer;
	Camera& camera;
	VkDescriptorSet globalDescriptorSet;
};
}
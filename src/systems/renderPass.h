#pragma once

#include "camera.h"
#include "pipeline.h"
#include "device.h"
#include "gameobject.h"
#include "frameInfo.h"
#include "descriptor.h"

#include <memory>
#include <vector>

namespace VulkanEngine
{
class RenderPass
{
public:
	RenderPass(Device& device, DescriptorPool& descriptorPool);
	~RenderPass();

	RenderPass(const RenderPass&) = delete;
	RenderPass& operator=(const RenderPass&) = delete;

	virtual void render(const FrameInfo& frameInfo) = 0;

private:
	virtual void createUniformBuffers() = 0;
	virtual void createDescriptorSetLayout() = 0;
	virtual void createDescriptorSets() = 0;
	virtual void createPipelineLayout() = 0;
	virtual void createPipeline() = 0;

protected:
	Device& device;
	DescriptorPool& descriptorPool;

	std::vector<std::unique_ptr<Buffer>> uniformBuffers;

	DescriptorSetLayout descriptorSetLayout{ device };

	std::vector<VkDescriptorSet> descriptorSets;

	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
};
}
#pragma once

#include "renderPass.h"

namespace VulkanEngine
{

class PointLightPass : public RenderPass
{
public:
	PointLightPass(Device& device, DescriptorPool& descriptorPool);
	~PointLightPass();

	PointLightPass(const PointLightPass&) = delete;
	PointLightPass& operator=(const PointLightPass&) = delete;

	void render(const FrameInfo& frameInfo);

private:
	virtual void createUniformBuffers() override;
	virtual void createDescriptorSetLayout() override;
	virtual void createDescriptorSets() override;
	virtual void createPipelineLayout() override;
	virtual void createPipeline() override;
};

}
#pragma once

#include "renderPass.h"

namespace VulkanEngine
{

class GameObjectPass : public RenderPass
{
public:
	GameObjectPass(Device& device, DescriptorPool& descriptorPool);
	~GameObjectPass();

	GameObjectPass(const GameObjectPass&) = delete;
	GameObjectPass& operator=(const GameObjectPass&) = delete;

	void update(FrameInfo& frameInfo);
	void render(const FrameInfo& frameInfo);

private:
	virtual void createUniformBuffers() override;
	virtual void createDescriptorSetLayout() override;
	virtual void createDescriptorSets() override;
	virtual void createPipelineLayout() override;
	virtual void createPipeline() override;
};

}
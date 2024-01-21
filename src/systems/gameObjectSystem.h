#pragma once

#include "camera.h"
#include "pipeline.h"
#include "device.h"
#include "gameobject.h"
#include "frameInfo.h"

#include <memory>
#include <vector>

namespace VulkanEngine
{

	class GameObjectSystem
	{
	public:
		GameObjectSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~GameObjectSystem();

		GameObjectSystem(const GameObjectSystem&) = delete;
		GameObjectSystem& operator=(const GameObjectSystem&) = delete;

		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		Device& device;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};

}
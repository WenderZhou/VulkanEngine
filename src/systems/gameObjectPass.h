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

	class GameObjectSystem
	{
	public:
		GameObjectSystem(Device& device, DescriptorPool& descriptorPool);
		~GameObjectSystem();

		GameObjectSystem(const GameObjectSystem&) = delete;
		GameObjectSystem& operator=(const GameObjectSystem&) = delete;

		void update(FrameInfo& frameInfo);
		void render(const FrameInfo& frameInfo);

	private:
		void createUniformBuffers();
		void createDescriptorSetLayout();
		void createDescriptorSets();
		void createPipelineLayout();
		void createPipeline();

		Device& device;
		DescriptorPool& descriptorPool;

		std::vector<std::unique_ptr<Buffer>> uniformBuffers;

		DescriptorSetLayout descriptorSetLayout{ device };

		std::vector<VkDescriptorSet> descriptorSets;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};

}
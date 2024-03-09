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
	class PointLightPass
	{
	public:
		PointLightPass(Device& device, DescriptorPool& descriptorPool);
		~PointLightPass();

		PointLightPass(const PointLightPass&) = delete;
		PointLightPass& operator=(const PointLightPass&) = delete;

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
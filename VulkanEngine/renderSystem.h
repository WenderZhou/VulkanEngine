#pragma once

#include "pipeline.h"
#include "device.h"
#include "gameobject.h"

#include <memory>
#include <vector>

namespace VulkanEngine
{

	class RenderSystem
	{
	public:
		RenderSystem(Device& device, VkRenderPass renderPass);
		~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;

		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		Device& device;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};

}
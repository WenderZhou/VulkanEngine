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

	class RenderSystem
	{
	public:
		RenderSystem(Device& device, VkRenderPass renderPass);
		~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo, std::vector<GameObject>& gameObjects);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		Device& device;

		std::unique_ptr<Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};

}
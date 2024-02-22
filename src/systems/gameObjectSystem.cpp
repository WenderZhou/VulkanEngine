#include "gameObjectSystem.h"
#include "model.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace VulkanEngine
{

struct SimplePushConstantData
{
	glm::mat4 modelMartix{ 1.0f };
	glm::mat4 normalMatrix{ 1.0f };
};

GameObjectSystem::GameObjectSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) :device{ device }
{
	createPipelineLayout(globalSetLayout);
	createPipeline(renderPass);
}

GameObjectSystem::~GameObjectSystem()
{
	vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
}

void GameObjectSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void GameObjectSystem::createPipeline(VkRenderPass renderPass)
{
	assert(pipelineLayout != nullptr && "Can not create pipeline before pipeline layout");

	PipelineConfig pipelineConfig{};
	pipelineConfig.bindingDescriptions = Model::Vertex::getBindingDescriptions();
	pipelineConfig.attributeDescriptions = Model::Vertex::getAttributeDescriptions();
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;

	pipeline = std::make_unique<Pipeline>(device, "shaders/basic.vert.spv", "shaders/basic.frag.spv", pipelineConfig);
}

void GameObjectSystem::render(FrameInfo& frameInfo)
{
	pipeline->bind(frameInfo.commandBuffer);

	glm::mat4 projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

	vkCmdBindDescriptorSets(
		frameInfo.commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout,
		0,
		1,
		&frameInfo.globalDescriptorSet,
		0,
		nullptr);

	for (auto& kv : frameInfo.gameObjects)
	{
		auto& obj = kv.second;

		if (obj.pModel == nullptr)
			continue;

		SimplePushConstantData push{};
		push.modelMartix = obj.transform.mat4();
		push.normalMatrix = obj.transform.normalMatrix();

		vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

		obj.pModel->bind(frameInfo.commandBuffer);
		obj.pModel->draw(frameInfo.commandBuffer);
	}
}

}
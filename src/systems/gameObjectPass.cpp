#include "gameObjectPass.h"
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

struct GameObjectUniformData
{
	glm::mat4 projection{ 1.0f };
	glm::mat4 view{ 1.0f };
	glm::mat4 invView{ 1.0f };
	glm::vec4 ambientLightColor{ 1.0f, 1.0f, 1.0f, 0.02f };
	PointLight pointLights[MAX_LIGHTS];
	int numLights;
};

GameObjectPass::GameObjectPass(Device& device, DescriptorPool& descriptorPool) : RenderPass(device, descriptorPool)
{
	createUniformBuffers();
	createDescriptorSetLayout();
	createDescriptorSets();
	createPipelineLayout();
	createPipeline();
}

GameObjectPass::~GameObjectPass()
{
	vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
}

void GameObjectPass::createUniformBuffers()
{
	uniformBuffers.resize(Device::MAX_FRAMES_IN_FLIGHT);
	for(int i = 0; i < uniformBuffers.size(); i++)
	{
		uniformBuffers[i] = std::make_unique<Buffer>(device, sizeof(GameObjectUniformData), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uniformBuffers[i]->map();
	}
}

void GameObjectPass::createDescriptorSetLayout()
{
	descriptorSetLayout.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS);
	descriptorSetLayout.build();
}

void GameObjectPass::createDescriptorSets()
{
	descriptorSets.resize(Device::MAX_FRAMES_IN_FLIGHT);
	for(int i = 0; i < descriptorSets.size(); i++)
	{
		auto bufferInfo = uniformBuffers[i]->descriptorInfo();
		std::vector<DescriptorDesc> descriptorDescs = { {0, &bufferInfo} };
		descriptorPool.allocateDescriptorSet(descriptorSetLayout, descriptorDescs, descriptorSets[i]);
	}
}

void GameObjectPass::createPipelineLayout()
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ descriptorSetLayout.getDescriptorSetLayout() };

	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	createInfo.pSetLayouts = descriptorSetLayouts.data();
	createInfo.pushConstantRangeCount = 1;
	createInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(device.getDevice(), &createInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void GameObjectPass::createPipeline()
{
	assert(pipelineLayout != nullptr && "Can not create pipeline before pipeline layout");

	PipelineConfig pipelineConfig{};
	pipelineConfig.bindingDescriptions = Model::Vertex::getBindingDescriptions();
	pipelineConfig.attributeDescriptions = Model::Vertex::getAttributeDescriptions();
	pipelineConfig.renderPass = device.getRenderPass();
	pipelineConfig.pipelineLayout = pipelineLayout;

	pipeline = std::make_unique<Pipeline>(device, "shaders/basic.vert.spv", "shaders/basic.frag.spv", pipelineConfig);
}

void GameObjectPass::update(FrameInfo& frameInfo)
{
	// rotate lights
	auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, { 0.f, -1.f, 0.f });
	for (auto& vk : frameInfo.gameObjects)
	{
		auto& obj = vk.second;
		if (obj.pPointLightComponent == nullptr)
			continue;

		obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));
	}
}

void GameObjectPass::render(const FrameInfo& frameInfo)
{
	GameObjectUniformData uniformData{};
	uniformData.projection = frameInfo.camera.getProjection();
	uniformData.view = frameInfo.camera.getView();
	uniformData.invView = frameInfo.camera.getInvView();
	
	int numLights = 0;
	for (auto& vk : frameInfo.gameObjects)
	{
		auto& obj = vk.second;
		if (obj.pPointLightComponent == nullptr)
			continue;

		uniformData.pointLights[numLights].position = glm::vec4(obj.transform.translation, 1.0f);
		uniformData.pointLights[numLights].color = glm::vec4(obj.color, obj.pPointLightComponent->lightIntensity);
		numLights++;
	}
	uniformData.numLights = numLights;

	uniformBuffers[frameInfo.frameIndex]->writeToBuffer(&uniformData);
	uniformBuffers[frameInfo.frameIndex]->flush();

	pipeline->bind(frameInfo.commandBuffer);

	glm::mat4 projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

	vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[frameInfo.frameIndex], 0, nullptr);

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
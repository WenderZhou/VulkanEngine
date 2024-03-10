#include "pointLightPass.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <stdexcept>
#include <map>

namespace VulkanEngine
{

struct PointLightPushConstant
{
	glm::vec4 position{};
	glm::vec4 color{};
	float radius;
};

struct PointLightUniformData
{
	glm::mat4 projection{ 1.0f };
	glm::mat4 view{ 1.0f };
};

PointLightPass::PointLightPass(Device& device, DescriptorPool& descriptorPool) : RenderPass(device, descriptorPool)
{
	createUniformBuffers();
	createDescriptorSetLayout();
	createDescriptorSets();
	createPipelineLayout();
	createPipeline();
}

PointLightPass::~PointLightPass()
{
	vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
}

void PointLightPass::createUniformBuffers()
{
	uniformBuffers.resize(Device::MAX_FRAMES_IN_FLIGHT);
	for(int i = 0; i < uniformBuffers.size(); i++)
	{
		uniformBuffers[i] = std::make_unique<Buffer>(device, sizeof(PointLightUniformData), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uniformBuffers[i]->map();
	}
}

void PointLightPass::createDescriptorSetLayout()
{
	descriptorSetLayout.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS);
	descriptorSetLayout.build();
}

void PointLightPass::createDescriptorSets()
{
	descriptorSets.resize(Device::MAX_FRAMES_IN_FLIGHT);
	for(int i = 0; i < descriptorSets.size(); i++)
	{
		auto bufferInfo = uniformBuffers[i]->descriptorInfo();
		std::vector<DescriptorDesc> descriptorDescs = { {0, &bufferInfo} };
		descriptorPool.allocateDescriptorSet(descriptorSetLayout, descriptorDescs, descriptorSets[i]);
	}
}

void PointLightPass::createPipelineLayout()
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PointLightPushConstant);

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

void PointLightPass::createPipeline()
{
	assert(pipelineLayout != nullptr && "Can not create pipeline before pipeline layout");

	PipelineConfig pipelineConfig{};
	pipelineConfig.renderPass = device.getRenderPass();
	pipelineConfig.pipelineLayout = pipelineLayout;

	pipeline = std::make_unique<Pipeline>(device, "shaders/pointLight.vert.spv", "shaders/pointLight.frag.spv", pipelineConfig);
}

void PointLightPass::render(const FrameInfo& frameInfo)
{
	PointLightUniformData ubo{};
	ubo.projection = frameInfo.camera.getProjection();
	ubo.view = frameInfo.camera.getView();

	uniformBuffers[frameInfo.frameIndex]->writeToBuffer(&ubo);
	uniformBuffers[frameInfo.frameIndex]->flush();

	std::map<float, GameObject::id_t> map;
	for (auto& vk : frameInfo.gameObjects)
	{
		auto& obj = vk.second;
		if(obj.pPointLightComponent != nullptr)
		{
			glm::vec3 diff = frameInfo.camera.getPosition() - obj.transform.translation;
			float dist2 = glm::dot(diff, diff);
			map[dist2] = obj.getId();
		}
	}

	pipeline->bind(frameInfo.commandBuffer);

	glm::mat4 projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

	vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[frameInfo.frameIndex], 0, nullptr);

	for (auto it = map.rbegin(); it != map.rend(); ++it)
	{
		auto& obj = frameInfo.gameObjects.at(it->second);

		PointLightPushConstant push{};
		push.position = glm::vec4(obj.transform.translation, 1.0f);
		push.color = glm::vec4(obj.color, obj.pPointLightComponent->lightIntensity);
		push.radius = obj.transform.scale.x;

		vkCmdPushConstants(
			frameInfo.commandBuffer,
			pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PointLightPushConstant),
			&push
		);

		vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
	}
}

}
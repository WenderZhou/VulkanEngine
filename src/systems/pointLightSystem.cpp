#include "pointLightSystem.h"

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

PointLightSystem::PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) :device{ device }
{
	createPipelineLayout(globalSetLayout);
	createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem()
{
	device.destroyPipelineLayout(pipelineLayout);
}

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PointLightPushConstant);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	device.createPipelineLayout(pipelineLayoutInfo, pipelineLayout);
}

void PointLightSystem::createPipeline(VkRenderPass renderPass)
{
	assert(pipelineLayout != nullptr && "Can not create pipeline before pipeline layout");

	PipelineConfig pipelineConfig{};
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;

	pipeline = std::make_unique<Pipeline>(device, "shaders/pointLight.vert.spv", "shaders/pointLight.frag.spv", pipelineConfig);
}

void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo)
{
	auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, { 0.f, -1.f, 0.f });
	int lightCount = 0;
	for (auto& vk : frameInfo.gameObjects)
	{
		auto& obj = vk.second;
		if (obj.pPointLightComponent == nullptr)
			continue;
		
		//obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));

		ubo.pointLights[lightCount].position = glm::vec4(obj.transform.translation, 1.0f);
		ubo.pointLights[lightCount].color = glm::vec4(obj.color, obj.pPointLightComponent->lightIntensity);
		lightCount++;
	}
	ubo.numLights = lightCount;
}

void PointLightSystem::render(FrameInfo& frameInfo)
{
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

	vkCmdBindDescriptorSets(
		frameInfo.commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout,
		0,
		1,
		&frameInfo.globalDescriptorSet,
		0,
		nullptr);

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
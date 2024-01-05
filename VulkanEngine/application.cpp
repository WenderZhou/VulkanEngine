#include "application.h"

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
	glm::mat2 transform{ 1.0f };
	glm::vec2 offset;
	alignas(16) glm::vec3 color;
};

App::App()
{
	loadGameObjects();
	createPipelineLayout();
	recreateSwapchain();
	createCommandBuffers();
}

App::~App()
{
	vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

void App::run()
{
	while (!window.shouldClose())
	{
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(device.device());
}

void App::loadGameObjects()
{
	std::vector<Model::Vertex> vertices{
		{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

	std::shared_ptr<Model> model = std::make_shared<Model>(device, vertices);

	GameObject triangle = GameObject::createGameObject();
	triangle.model = model;
	triangle.color = { 0.1f, 0.8f, 0.1f };
	triangle.transform2d.translation.x = 0.2f;
	triangle.transform2d.scale = { 2.0f, 0.5f };
	triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();

	gameObjects.push_back(std::move(triangle));
}

void App::createPipelineLayout()
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void App::createPipeline()
{
	assert(swapchain != nullptr && "Can not create pipeline before swap chain");
	assert(pipelineLayout != nullptr && "Can not create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfigInfo{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfigInfo);
	pipelineConfigInfo.renderPass = swapchain->getRenderPass();
	pipelineConfigInfo.pipelineLayout = pipelineLayout;

	pipeline = std::make_unique<Pipeline>(device, "shaders/basic.vert.spv", "shaders/basic.frag.spv", pipelineConfigInfo);
}

void App::createCommandBuffers()
{
	commandBuffers.resize(swapchain->imageCount());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = device.getCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command buffer");
	}
}

void App::recreateSwapchain()
{
	auto extent = window.getExtent();
	while (extent.width == 0 || extent.height == 0)
	{
		extent = window.getExtent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device.device());

	if (swapchain == nullptr)
	{
		swapchain = std::make_unique<Swapchain>(device, extent);
	}
	else
	{
		swapchain = std::make_unique<Swapchain>(device, extent, std::move(swapchain));
		if (swapchain->imageCount() != commandBuffers.size())
		{
			freeCommandBuffers();
			createCommandBuffers();
		}
	}
	
	createPipeline();
}

void App::freeCommandBuffers()
{
	vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	commandBuffers.clear();
}

void App::recordCommandBuffer(int imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer");
	}

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = swapchain->getRenderPass();
	renderPassInfo.framebuffer = swapchain->getFrameBuffer(imageIndex);
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = swapchain->getSwapChainExtent();
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(swapchain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0, 0}, swapchain->getSwapChainExtent() };
	vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

	pipeline->bind(commandBuffers[imageIndex]);

	renderGameObjects(commandBuffers[imageIndex]);

	vkCmdEndRenderPass(commandBuffers[imageIndex]);
	if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer");
	}
}

void App::renderGameObjects(VkCommandBuffer commandBuffer)
{
	pipeline->bind(commandBuffer);

	for (auto& obj : gameObjects)
	{
		obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.001f, glm::two_pi<float>());

		SimplePushConstantData push{};
		push.offset = obj.transform2d.translation;
		push.color = obj.color;
		push.transform = obj.transform2d.mat2();
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

		obj.model->bind(commandBuffer);
		obj.model->draw(commandBuffer);
	}
}

void App::drawFrame()
{
	uint32_t imageIndex;
	VkResult result = swapchain->acquireNextImage(&imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain();
		return;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image");
	}

	recordCommandBuffer(imageIndex);
	result = swapchain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized())
	{
		window.resetWindowResizedFlag();
		recreateSwapchain();
		return;
	}

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image");
	}
}

}
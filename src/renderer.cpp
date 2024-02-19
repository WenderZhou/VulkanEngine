#include "renderer.h"

#include <array>
#include <cassert>
#include <stdexcept>

namespace VulkanEngine
{

Renderer::Renderer(Window& window, Device& device) :
	window{ window }, device{ device }, currentFrameIndex{ 0 }, isFrameStarted{ false }
{
	recreateSwapchain();
	createCommandBuffers();
}

Renderer::~Renderer()
{
	freeCommandBuffers();
}

void Renderer::createCommandBuffers()
{
	commandBuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = device.getCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	if(vkAllocateCommandBuffers(device.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command buffer");
	}
}

void Renderer::recreateSwapchain()
{
	auto extent = window.getExtent();
	while(extent.width == 0 || extent.height == 0)
	{
		extent = window.getExtent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device.getDevice());

	if(swapchain == nullptr)
	{
		swapchain = std::make_unique<Swapchain>(device, extent);
	}
	else
	{
		std::shared_ptr<Swapchain> oldSwapchain = std::move(swapchain);
		swapchain = std::make_unique<Swapchain>(device, extent, oldSwapchain);

		if(!oldSwapchain->compareSwapFormats(*swapchain.get()))
		{
			throw std::runtime_error("Swap chain image(or depth) format has changed");
		}
	}
}

void Renderer::freeCommandBuffers()
{
	vkFreeCommandBuffers(device.getDevice(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	commandBuffers.clear();
}

VkCommandBuffer Renderer::beginFrame()
{
	assert(!isFrameStarted && "Can't call beginFrame while already in progress");

	VkResult result = swapchain->acquireNextImage(&currentImageIndex);

	if(result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain();
		return nullptr;
	}

	if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image");
	}

	isFrameStarted = true;

	VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer");
	}

	return commandBuffer;
}

void Renderer::endFrame()
{
	assert(isFrameStarted && "Can't call endFrame while frame is not in progress");

	VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

	if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer");
	}

	VkResult result = swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized())
	{
		window.resetWindowResizedFlag();
		recreateSwapchain();
	}
	else if(result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image");
	}

	isFrameStarted = false;
	currentFrameIndex = (currentFrameIndex + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapchainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(isFrameStarted && "Can't call beginSwapchainRenderPass while frame is not in progress");
	assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = swapchain->getRenderPass();
	renderPassInfo.framebuffer = swapchain->getFrameBuffer(currentImageIndex);
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = swapchain->getSwapChainExtent();
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(swapchain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0, 0}, swapchain->getSwapChainExtent() };
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endSwapchainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(isFrameStarted && "Can't call endSwapchainRenderPass while frame is not in progress");
	assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

	vkCmdEndRenderPass(commandBuffer);
}

}
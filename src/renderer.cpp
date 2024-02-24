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
	m_commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	device.allocateCommandBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT, m_commandBuffers.data());
}

void Renderer::freeCommandBuffers()
{
	device.freeCommandBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT, m_commandBuffers.data());
	m_commandBuffers.clear();
}

void Renderer::recreateSwapchain()
{
	auto extent = window.getExtent();
	while(extent.width == 0 || extent.height == 0)
	{
		extent = window.getExtent();
		glfwWaitEvents();
	}

	device.waitIdle();

	if(m_pSwapChain == nullptr)
	{
		m_pSwapChain = std::make_unique<SwapChain>(device, extent);
	}
	else
	{
		std::shared_ptr<SwapChain> oldSwapchain = std::move(m_pSwapChain);
		m_pSwapChain = std::make_unique<SwapChain>(device, extent, oldSwapchain);

		if(!oldSwapchain->compareSwapFormats(*m_pSwapChain.get()))
		{
			throw std::runtime_error("Swap chain image(or depth) format has changed");
		}
	}
}

VkCommandBuffer Renderer::beginFrame()
{
	assert(!isFrameStarted && "Can't call beginFrame while already in progress");

	VkResult result = m_pSwapChain->acquireNextImage(&currentImageIndex);

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

	VkResult result = m_pSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
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
	currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapchainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(isFrameStarted && "Can't call beginSwapchainRenderPass while frame is not in progress");
	assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = m_pSwapChain->getRenderPass();
	beginInfo.framebuffer = m_pSwapChain->getFrameBuffer(currentImageIndex);
	beginInfo.renderArea.offset = { 0,0 };
	beginInfo.renderArea.extent = m_pSwapChain->getSwapChainExtent();
	beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	beginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_pSwapChain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(m_pSwapChain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0, 0}, m_pSwapChain->getSwapChainExtent() };
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
#pragma once

#include "device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace VulkanEngine {

class SwapChain {
public:
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	SwapChain(Device& deviceRef, VkExtent2D windowExtent);
	SwapChain(Device& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> pOldSwapChain);
	~SwapChain();

	SwapChain(const SwapChain&) = delete;
	SwapChain& operator=(const SwapChain&) = delete;

	VkFramebuffer getFrameBuffer(int index) { return m_swapChainFramebuffers[index]; }
	VkRenderPass getRenderPass() { return m_renderPass; }
	VkImageView getImageView(int index) { return m_swapChainImageViews[index]; }
	VkFormat getSwapChainImageFormat() { return m_swapChainImageFormat; }
	VkExtent2D getSwapChainExtent() { return m_swapChainExtent; }

	float extentAspectRatio() { return static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height); }
	VkFormat findDepthFormat();

	VkResult acquireNextImage(uint32_t* imageIndex);
	VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

	bool compareSwapFormats(const SwapChain& swapChain) const
	{
		return swapChain.m_swapChainDepthFormat == m_swapChainDepthFormat && swapChain.m_swapChainImageFormat == m_swapChainImageFormat;
	}

private:
	void init();
	void createSwapChain();
	void createImageViews();
	void createDepthResources();
	void createRenderPass();
	void createFramebuffers();
	void createSyncObjects();

	// Helper functions
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkFormat m_swapChainImageFormat;
	VkFormat m_swapChainDepthFormat;
	VkExtent2D m_swapChainExtent;

	std::vector<VkFramebuffer> m_swapChainFramebuffers;
	VkRenderPass m_renderPass;

	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImage> depthImages;
	std::vector<VkDeviceMemory> depthImageMemorys;
	std::vector<VkImageView> depthImageViews;
	std::vector<VkImageView> m_swapChainImageViews;

	Device& m_device;
	VkExtent2D m_windowExtent;

	VkSwapchainKHR m_swapChain;
	std::shared_ptr<SwapChain> m_pOldSwapchain;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;
};

}

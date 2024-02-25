#pragma once

#include "device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace VulkanEngine {

class Swapchain {
public:
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	Swapchain(Device& deviceRef, VkExtent2D windowExtent);
	Swapchain(Device& deviceRef, VkExtent2D windowExtent, std::shared_ptr<Swapchain> pOldSwapChain);
	~Swapchain();

	Swapchain(const Swapchain&) = delete;
	Swapchain& operator=(const Swapchain&) = delete;

	VkFramebuffer getFrameBuffer(int index) { return m_swapchainFramebuffers[index]; }
	VkRenderPass getRenderPass() { return m_renderPass; }
	VkImageView getImageView(int index) { return m_swapchainImageViews[index]; }
	VkFormat getSwapChainImageFormat() { return m_swapchainImageFormat; }
	VkExtent2D getSwapChainExtent() { return m_swapchainExtent; }

	float extentAspectRatio() { return static_cast<float>(m_swapchainExtent.width) / static_cast<float>(m_swapchainExtent.height); }
	VkFormat findDepthFormat();

	VkResult acquireNextImage(uint32_t* imageIndex);
	VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

	bool compareSwapFormats(const Swapchain& swapChain) const
	{
		return swapChain.m_swapchainDepthFormat == m_swapchainDepthFormat && swapChain.m_swapchainImageFormat == m_swapchainImageFormat;
	}

private:
	void init();
	void createSwapchain();
	void createImageViews();
	void createDepthResources();
	void createRenderPass();
	void createFramebuffers();
	void createSyncObjects();

	// Helper functions
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkFormat m_swapchainImageFormat;
	VkFormat m_swapchainDepthFormat;
	VkExtent2D m_swapchainExtent;

	std::vector<VkFramebuffer> m_swapchainFramebuffers;
	VkRenderPass m_renderPass;

	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImage> depthImages;
	std::vector<VkDeviceMemory> depthImageMemorys;
	std::vector<VkImageView> depthImageViews;
	std::vector<VkImageView> m_swapchainImageViews;

	Device& m_device;
	VkExtent2D m_windowExtent;

	VkSwapchainKHR m_swapchain;
	std::shared_ptr<Swapchain> m_pOldSwapchain;

	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
	std::vector<VkFence> m_imagesInFlightFences;
	size_t currentFrame = 0;
};

}

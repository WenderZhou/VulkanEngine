#pragma once

#include "window.h"

// std lib headers
#include <string>
#include <vector>
#include <optional>
#include <cassert>

namespace VulkanEngine
{

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

class Device
{
public:
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	Device(Window& window);
	~Device();

	// Not copyable or movable
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;
	Device(Device&&) = delete;
	Device& operator=(Device&&) = delete;

	VkInstance getInstance() { return m_instance; }

	VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; }
	VkDevice getDevice() { return m_device; }

	VkQueue getGraphicsQueue() { return m_graphicsQueue; }
	VkQueue getPresentQueue() { return m_presentQueue; }

	VkCommandPool getCommandPool() { return m_commandPool; }

	VkRenderPass getRenderPass() { return m_renderPass; }

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	QueueFamilyIndices getQueueFamilyIndices() { return getQueueFamilyIndices(m_physicalDevice); }
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	// Buffer Helper Functions
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

	void waitIdle();

	VkImageView getImageView(int index) { return m_swapchainImageViews[index]; }

	float getAspectRatio() { return static_cast<float>(m_swapchainExtent.width) / static_cast<float>(m_swapchainExtent.height); }
	VkFormat findDepthFormat();

	VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

	int getFrameIndex() const{return currentFrameIndex;}

	VkCommandBuffer beginFrame();
	void endFrame();

	void beginRenderPass(VkCommandBuffer commandBuffer);
	void endRenderPass(VkCommandBuffer commandBuffer);

private:
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapchain();
	void createImageViews();
	void createDepthResources();
	void createRenderPass();
	void createFramebuffers();
	void createSyncObjects();
	void createCommandPool();

	void createCommandBuffers();
	void freeCommandBuffers();
	void recreateSwapchain();

	void cleanupSwapchain();

	// helper functions
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice physicalDevice);

	bool isSuitablePhysicalDevice(VkPhysicalDevice physicalDevice);

	std::vector<const char*> getRequiredInstanceExtensions();
	std::vector<const char*> getRequiredInstanceLayers();
	std::vector<const char*> getRequiredDeviceExtensions();

	SwapChainSupportDetails getSwapChainSupportDetails(VkPhysicalDevice physicalDevice);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	Window& m_window;

	VkInstance m_instance = VK_NULL_HANDLE;

	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;	// physical device handle
	VkDevice m_device = VK_NULL_HANDLE;					// logical device handle

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	VkCommandPool m_commandPool;

	VkDebugUtilsMessengerEXT m_debugMessenger;

	VkSurfaceKHR m_surface;

	VkFormat m_swapchainImageFormat;
	VkFormat m_swapchainDepthFormat;
	VkExtent2D m_swapchainExtent;

	std::vector<VkFramebuffer> m_swapchainFramebuffers;
	VkRenderPass m_renderPass;

	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;

	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;

	VkExtent2D m_windowExtent;

	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
	std::vector<VkFence> m_imagesInFlightFences;
	size_t currentFrame = 0;

	std::vector<VkCommandBuffer> m_commandBuffers;

	uint32_t currentImageIndex;
	int currentFrameIndex = 0;
};

}
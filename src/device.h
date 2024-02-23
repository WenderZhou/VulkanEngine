#pragma once

#include "window.h"

// std lib headers
#include <string>
#include <vector>
#include <optional>

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
	VkSurfaceKHR getSurface() { return m_surface; }

	SwapChainSupportDetails getSwapChainSupportDetails() { return getSwapChainSupportDetails(m_physicalDevice); }
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	QueueFamilyIndices getQueueFamilyIndices() { return getQueueFamilyIndices(m_physicalDevice); }
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	// Buffer Helper Functions
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

	void allocateMemory(const VkMemoryAllocateInfo& allocateInfo, VkDeviceMemory& memory);
	void freeMemory(VkDeviceMemory memory);

	void createImage(const VkImageCreateInfo& createInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void destroyImage(VkImage image);

	void createImageView(const VkImageViewCreateInfo& createInfo, VkImageView& imageView);
	void destroyImageView(VkImageView imageView);

	void createShaderModule(const VkShaderModuleCreateInfo& createInfo, VkShaderModule& shaderModule);
	void destroyShaderModule(VkShaderModule shaderModule);

	void createGraphicsPipeline(const VkGraphicsPipelineCreateInfo& createInfo, VkPipeline& pipeline);
	void destroyPipeline(VkPipeline pipeline);

	void createFramebuffer(const VkFramebufferCreateInfo& createInfo, VkFramebuffer& framebuffer);
	void destroyFramebuffer(VkFramebuffer framebuffer);

	void createRenderPass(const VkRenderPassCreateInfo& createInfo, VkRenderPass& renderPass);
	void destroyRenderPass(VkRenderPass renderPass);

	void createCommandPool(const VkCommandPoolCreateInfo& createInfo, VkCommandPool& commandPool);
	void destroyCommandPool(VkCommandPool commandPool);

	void allocateCommandBuffers(const VkCommandBufferAllocateInfo& allocateInfo, VkCommandBuffer* pCommandBuffers);
	void freeCommandBuffers(VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);

	VkPhysicalDeviceProperties properties;

	void waitIdle();

private:
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createCommandPool();

	// helper functions
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice physicalDevice);

	bool isSuitablePhysicalDevice(VkPhysicalDevice physicalDevice);

	std::vector<const char*> getRequiredInstanceExtensionNames();
	void checkInstanceExtensionSupport();

	std::vector<const char*> getRequiredDeviceExtensionNames();
	bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice);

	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	bool checkValidationLayerSupport();

	SwapChainSupportDetails getSwapChainSupportDetails(VkPhysicalDevice physicalDevice);

	VkInstance m_instance = VK_NULL_HANDLE;

	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;	// physical device handle
	VkDevice m_device = VK_NULL_HANDLE;					// logical device handle

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	Window& m_window;
	VkCommandPool m_commandPool;

	VkDebugUtilsMessengerEXT m_debugMessenger;

	VkSurfaceKHR m_surface;
};

}
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

	VkCommandPool getCommandPool() { return commandPool; }
	VkSurfaceKHR surface() { return m_surface; }

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

	void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

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
	bool isSuitablePhysicalDevice(VkPhysicalDevice physicalDevice);
	std::vector<const char*> getRequiredInstanceExtensions();
	bool checkValidationLayerSupport();
	QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice device);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void hasGflwRequiredInstanceExtensions();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails getSwapChainSupportDetails(VkPhysicalDevice device);

	VkInstance m_instance = VK_NULL_HANDLE;
	
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;	// physical device handle
	VkDevice m_device = VK_NULL_HANDLE;					// logical device handle

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	Window& m_window;
	VkCommandPool commandPool;

	VkDebugUtilsMessengerEXT debugMessenger;

	VkSurfaceKHR m_surface;

	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

}
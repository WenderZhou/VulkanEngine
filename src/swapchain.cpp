#include "swapchain.h"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace VulkanEngine
{

Swapchain::Swapchain(Device& deviceRef, VkExtent2D windowExtent)
	: m_device{ deviceRef }, m_windowExtent{ windowExtent } {
	init();
}

Swapchain::Swapchain(Device& deviceRef, VkExtent2D windowExtent, std::shared_ptr<Swapchain> pOldSwapChain)
	: m_device{ deviceRef }, m_windowExtent{ windowExtent }, m_pOldSwapchain{ pOldSwapChain }
{
	init();

	// clean up old swap chain since it's no longer needed
	m_pOldSwapchain = nullptr;
}

void Swapchain::init()
{
	createSwapchain();
	createImageViews();
	createRenderPass();
	createDepthResources();
	createFramebuffers();
	createSyncObjects();
}

Swapchain::~Swapchain()
{
	for(auto imageView : m_swapchainImageViews)
	{
		m_device.destroyImageView(imageView);
	}
	m_swapchainImageViews.clear();

	if(m_swapchain != VK_NULL_HANDLE)
	{
		m_device.destroySwapchain(m_swapchain);
		m_swapchain = VK_NULL_HANDLE;
	}

	for(int i = 0; i < depthImages.size(); i++)
	{
		m_device.destroyImageView(depthImageViews[i]);
		m_device.destroyImage(depthImages[i]);
		m_device.freeMemory(depthImageMemorys[i]);
	}

	for(auto framebuffer : m_swapchainFramebuffers)
	{
		m_device.destroyFramebuffer(framebuffer);
	}

	m_device.destroyRenderPass(m_renderPass);

	// cleanup synchronization objects
	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_device.destroySemaphore(m_renderFinishedSemaphores[i]);
		m_device.destroySemaphore(m_imageAvailableSemaphores[i]);
		m_device.destroyFence(m_inFlightFences[i]);
	}
}

VkResult Swapchain::acquireNextImage(uint32_t* imageIndex)
{
	m_device.waitForFence(m_inFlightFences[currentFrame]);

	VkResult result = m_device.acquireNextImage(m_swapchain, m_imageAvailableSemaphores[currentFrame], imageIndex);

	return result;
}

VkResult Swapchain::submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex)
{
	if(m_imagesInFlightFences[*imageIndex] != VK_NULL_HANDLE)
	{
		m_device.waitForFence(m_imagesInFlightFences[*imageIndex]);
	}
	m_imagesInFlightFences[*imageIndex] = m_inFlightFences[currentFrame];

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = buffers;

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	m_device.resetFence(m_inFlightFences[currentFrame]);

	m_device.graphicsQueueSubmit(submitInfo, m_inFlightFences[currentFrame]);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = imageIndex;

	VkResult result = m_device.present(presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return result;
}

void Swapchain::createSwapchain()
{
	SwapChainSupportDetails details = m_device.getSwapChainSupportDetails();

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(details.presentModes);
	VkExtent2D extent = chooseSwapExtent(details.capabilities);

	uint32_t imageCount = details.capabilities.minImageCount + 1;
	if(details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
	{
		imageCount = details.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_device.getSurface();

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = m_device.getQueueFamilyIndices();
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if(indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;      // Optional
		createInfo.pQueueFamilyIndices = nullptr;  // Optional
	}

	createInfo.preTransform = details.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = m_pOldSwapchain == nullptr ? VK_NULL_HANDLE : m_pOldSwapchain->m_swapchain;

	m_device.createSwapchain(createInfo, m_swapchain);

	m_device.getSwapchainImages(m_swapchain, m_swapchainImages);

	m_swapchainImageFormat = surfaceFormat.format;
	m_swapchainExtent = extent;
}

void Swapchain::createImageViews()
{
	m_swapchainImageViews.resize(m_swapchainImages.size());
	for(size_t i = 0; i < m_swapchainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapchainImageFormat;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		m_device.createImageView(createInfo, m_swapchainImageViews[i]);
	}
}

void Swapchain::createRenderPass()
{
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstSubpass = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &dependency;

	m_device.createRenderPass(createInfo, m_renderPass);
}

void Swapchain::createFramebuffers()
{
	m_swapchainFramebuffers.resize(m_swapchainImages.size());
	for(size_t i = 0; i < m_swapchainImages.size(); i++)
	{
		std::array<VkImageView, 2> attachments = { m_swapchainImageViews[i], depthImageViews[i] };

		VkExtent2D swapChainExtent = getSwapChainExtent();
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		m_device.createFramebuffer(framebufferInfo, m_swapchainFramebuffers[i]);
	}
}

void Swapchain::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();
	m_swapchainDepthFormat = depthFormat;
	VkExtent2D swapChainExtent = getSwapChainExtent();

	depthImages.resize(m_swapchainImages.size());
	depthImageMemorys.resize(m_swapchainImages.size());
	depthImageViews.resize(m_swapchainImages.size());

	for(int i = 0; i < depthImages.size(); i++)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = swapChainExtent.width;
		imageInfo.extent.height = swapChainExtent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = 0;

		m_device.createImage(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImages[i], depthImageMemorys[i]);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = depthImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		m_device.createImageView(viewInfo, depthImageViews[i]);
	}
}

void Swapchain::createSyncObjects()
{
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	m_imagesInFlightFences.resize(m_swapchainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_device.createSemaphore(semaphoreInfo, m_imageAvailableSemaphores[i]);
		m_device.createSemaphore(semaphoreInfo, m_renderFinishedSemaphores[i]);

		m_device.createFence(fenceInfo, m_inFlightFences[i]);
	}
}

VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for(const auto& availableFormat : availableFormats)
	{
		if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for(const auto& availablePresentMode : availablePresentModes)
	{
		if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			std::cout << "Present mode: Mailbox" << std::endl;
			return availablePresentMode;
		}
	}

	std::cout << "Present mode: V-Sync" << std::endl;
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = m_windowExtent;
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

VkFormat Swapchain::findDepthFormat()
{
	return m_device.findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

}

#pragma once

#include <string>
#include "device.h"

namespace VulkanEngine
{

class Image
{
public:
	Image(Device& device, const std::string& filepath);
	~Image();

	VkImageView getImageView() const { return m_imageView; }
	VkSampler getSampler() const { return m_sampler; }

	VkDescriptorImageInfo getImageInfo() const { return { m_sampler, m_imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }; }

private:
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
	void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void createImageView();
	void createSampler();

	Device& m_device;

	VkImage m_image;
	VkDeviceMemory m_imageMemory;
	VkImageView m_imageView;
	VkSampler m_sampler;
};

}
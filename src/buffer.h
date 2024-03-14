#pragma once

#include "device.h"

namespace VulkanEngine
{

class Buffer
{
public:
	Buffer(Device& device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
	~Buffer();

	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	void unmap();

	void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkDescriptorBufferInfo getBufferInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

	void writeToIndex(void* data, int index);
	VkResult flushIndex(int index);
	VkDescriptorBufferInfo descriptorInfoForIndex(int index);
	VkResult invalidateIndex(int index);

	VkBuffer getBuffer() const { return m_buffer; }
	void* getMappedMemory() const { return m_pData; }
	uint32_t getInstanceCount() const { return instanceCount; }
	VkDeviceSize getInstanceSize() const { return instanceSize; }
	VkDeviceSize getAlignmentSize() const { return instanceSize; }
	VkDeviceSize getBufferSize() const { return bufferSize; }

private:
	static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

	Device& m_device;
	void* m_pData = nullptr;
	VkBuffer m_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;

	VkDeviceSize bufferSize;
	uint32_t instanceCount;
	VkDeviceSize instanceSize;
	VkDeviceSize alignmentSize;
};

}
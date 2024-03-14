#pragma once

#include "device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace VulkanEngine
{

class DescriptorSetLayout {
public:
	DescriptorSetLayout(Device& device);
	~DescriptorSetLayout();

	DescriptorSetLayout(const DescriptorSetLayout&) = delete;
	DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

	void addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags);
	void build();

	VkDescriptorType getDescriptorType(uint32_t binding) const;

	VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

private:
	Device& device;
	VkDescriptorSetLayout descriptorSetLayout;
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

	friend class DescriptorPool;
};

struct DescriptorDesc
{
	uint32_t binding;
	union
	{
		VkDescriptorBufferInfo* pBufferInfo;
		VkDescriptorImageInfo* pImageInfo;
	};
};

class DescriptorPool {
public:
	DescriptorPool(Device& device);
	~DescriptorPool();
	DescriptorPool(const DescriptorPool&) = delete;
	DescriptorPool& operator=(const DescriptorPool&) = delete;

	void addPoolSize(VkDescriptorType descriptorType, uint32_t count);
	void setPoolFlags(VkDescriptorPoolCreateFlags flags);
	void setMaxSets(uint32_t count);
	void build();

	void allocateDescriptorSet(const DescriptorSetLayout& descriptorSetLayout, std::vector<DescriptorDesc>& descriptorDescs, VkDescriptorSet& descriptorSet);

	void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

	void resetPool();

	VkDescriptorPool getDescriptorPool();

private:
	Device& m_device;
	VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

	std::vector<VkDescriptorPoolSize> poolSizes{};
	uint32_t maxSets = 1000;
	VkDescriptorPoolCreateFlags poolFlags = 0;

	friend class DescriptorWriter;
};

}
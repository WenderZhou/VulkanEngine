#include "descriptor.h"

// std
#include <cassert>
#include <stdexcept>

namespace VulkanEngine
{

DescriptorSetLayout::DescriptorSetLayout(Device& device) : device{ device }
{

}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(device.getDevice(), descriptorSetLayout, nullptr);
}

void DescriptorSetLayout::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags)
{
	assert(bindings.count(binding) == 0 && "Binding already in use");
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = descriptorType;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = stageFlags;
	bindings[binding] = layoutBinding;
}

void DescriptorSetLayout::build()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
	for(auto kv : bindings)
	{
		setLayoutBindings.push_back(kv.second);
	}

	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	createInfo.pBindings = setLayoutBindings.data();

	if(vkCreateDescriptorSetLayout(device.getDevice(), &createInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

VkDescriptorType DescriptorSetLayout::getDescriptorType(uint32_t binding) const
{
	auto it = bindings.find(binding);
	if(it != bindings.end())
	{
		return it->second.descriptorType;
	}
	else
	{
		throw std::runtime_error("Layout does not contain specified binding");
	}
}

void DescriptorPool::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
{
	poolSizes.push_back({ descriptorType, count });
}

void DescriptorPool::setPoolFlags(VkDescriptorPoolCreateFlags flags)
{
	poolFlags = flags;
}

void DescriptorPool::setMaxSets(uint32_t count)
{
	maxSets = count;
}

void DescriptorPool::build()
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = maxSets;
	descriptorPoolInfo.flags = poolFlags;

	if(vkCreateDescriptorPool(m_device.getDevice(), &descriptorPoolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

DescriptorPool::DescriptorPool(Device& device) : m_device{ device }
{

}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(m_device.getDevice(), m_descriptorPool, nullptr);
}

void DescriptorPool::allocateDescriptorSet(const DescriptorSetLayout& descriptorSetLayout, std::vector<DescriptorDesc>& descriptorDescs, VkDescriptorSet& descriptorSet)
{
	std::vector<VkWriteDescriptorSet> writes;

	VkDescriptorSetLayout setLayout = descriptorSetLayout.getDescriptorSetLayout();

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.pSetLayouts = &setLayout;
	allocInfo.descriptorSetCount = 1;

	if(vkAllocateDescriptorSets(m_device.getDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor set");
	}

	for(DescriptorDesc& desc : descriptorDescs)
	{
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = descriptorSetLayout.getDescriptorType(desc.binding);
		write.dstBinding = desc.binding;
		write.pImageInfo = desc.pImageInfo;
		write.pBufferInfo = desc.pBufferInfo;
		write.descriptorCount = 1;
		write.dstSet = descriptorSet;

		writes.push_back(write);
	}

	vkUpdateDescriptorSets(m_device.getDevice(), writes.size(), writes.data(), 0, nullptr);
}

void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
{
	vkFreeDescriptorSets(m_device.getDevice(), m_descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
}

void DescriptorPool::resetPool()
{
	vkResetDescriptorPool(m_device.getDevice(), m_descriptorPool, 0);
}

VkDescriptorPool DescriptorPool::getDescriptorPool()
{
	return m_descriptorPool;
}

}
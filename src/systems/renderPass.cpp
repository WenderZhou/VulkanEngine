#include "renderPass.h"

namespace VulkanEngine
{

RenderPass::RenderPass(Device& device, DescriptorPool& descriptorPool) :device{ device }, descriptorPool{ descriptorPool }
{
	
}

RenderPass::~RenderPass()
{
	
}

}

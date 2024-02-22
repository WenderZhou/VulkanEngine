#pragma once

#include "device.h"
#include "shader.h"

#include <string>
#include <vector>
#include <memory>

namespace VulkanEngine 
{

struct PipelineConfig
{
	PipelineConfig() = default;

	PipelineConfig(const PipelineConfig&) = delete;
	PipelineConfig& operator=(const PipelineConfig&) = delete;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class Pipeline
{
public:
	Pipeline(Device& device, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfig& config);
	~Pipeline();

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

	void bind(VkCommandBuffer commandBuffer);
	
private:

	void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfig& config);

	Device& m_device;
	VkPipeline m_graphicsPipeline;

	std::unique_ptr<Shader> m_vertShader;
	std::unique_ptr<Shader> m_fragShader;

};

}
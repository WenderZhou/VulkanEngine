#include "pipeline.h"

#include "model.h"

#include <cassert>
#include <stdexcept>

namespace VulkanEngine
{

Pipeline::Pipeline(Device& device, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfig& config) :m_device(device)
{
	createGraphicsPipeline(vertFilepath, fragFilepath, config);
}

Pipeline::~Pipeline()
{
	vkDestroyPipeline(m_device.getDevice(), graphicsPipeline, nullptr);
}

void Pipeline::createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfig& config)
{
	assert(config.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipelineLayout provided in config");
	assert(config.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no renderPass provided in config");

	m_vertShader = std::make_unique<Shader>(m_device, vertFilepath);
	m_fragShader = std::make_unique<Shader>(m_device, fragFilepath);

	VkPipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = m_vertShader->getShaderModule();
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = m_fragShader->getShaderModule();
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].pSpecializationInfo = nullptr;

	auto& bindingDescriptions = config.bindingDescriptions;
	auto& attributeDescriptions = config.attributeDescriptions;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.stageCount = 2;
	createInfo.pStages = shaderStages;
	createInfo.pVertexInputState = &vertexInputInfo;
	createInfo.pInputAssemblyState = &config.inputAssemblyState;
	createInfo.pViewportState = &config.viewportState;
	createInfo.pRasterizationState = &config.rasterizationState;
	createInfo.pMultisampleState = &config.multisampleState;
	createInfo.pColorBlendState = &config.colorBlendState;
	createInfo.pDepthStencilState = &config.depthStencilState;
	createInfo.pDynamicState = &config.dynamicState;

	createInfo.layout = config.pipelineLayout;
	createInfo.renderPass = config.renderPass;
	createInfo.subpass = config.subpass;

	createInfo.basePipelineIndex = -1;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;

	if(vkCreateGraphicsPipelines(m_device.getDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline");
	}
}

void Pipeline::bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}

void Pipeline::defaultPipelineConfig(PipelineConfig& config)
{
	config.inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	config.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	config.inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	config.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	config.viewportState.viewportCount = 1;
	config.viewportState.pViewports = nullptr;
	config.viewportState.scissorCount = 1;
	config.viewportState.pScissors = nullptr;

	config.rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	config.rasterizationState.depthClampEnable = VK_FALSE;
	config.rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	config.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	config.rasterizationState.lineWidth = 1.0f;
	config.rasterizationState.cullMode = VK_CULL_MODE_NONE;
	config.rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	config.rasterizationState.depthBiasEnable = VK_FALSE;
	config.rasterizationState.depthBiasConstantFactor = 0.0f;  // Optional
	config.rasterizationState.depthBiasClamp = 0.0f;           // Optional
	config.rasterizationState.depthBiasSlopeFactor = 0.0f;     // Optional

	config.multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	config.multisampleState.sampleShadingEnable = VK_FALSE;
	config.multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	config.multisampleState.minSampleShading = 1.0f;           // Optional
	config.multisampleState.pSampleMask = nullptr;             // Optional
	config.multisampleState.alphaToCoverageEnable = VK_FALSE;  // Optional
	config.multisampleState.alphaToOneEnable = VK_FALSE;       // Optional

	config.colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	config.colorBlendAttachment.blendEnable = VK_FALSE;
	config.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
	config.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
	config.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
	config.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
	config.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
	config.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

	config.colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	config.colorBlendState.logicOpEnable = VK_FALSE;
	config.colorBlendState.logicOp = VK_LOGIC_OP_COPY;  // Optional
	config.colorBlendState.attachmentCount = 1;
	config.colorBlendState.pAttachments = &config.colorBlendAttachment;
	config.colorBlendState.blendConstants[0] = 0.0f;  // Optional
	config.colorBlendState.blendConstants[1] = 0.0f;  // Optional
	config.colorBlendState.blendConstants[2] = 0.0f;  // Optional
	config.colorBlendState.blendConstants[3] = 0.0f;  // Optional

	config.depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	config.depthStencilState.depthTestEnable = VK_TRUE;
	config.depthStencilState.depthWriteEnable = VK_TRUE;
	config.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	config.depthStencilState.depthBoundsTestEnable = VK_FALSE;
	config.depthStencilState.minDepthBounds = 0.0f;  // Optional
	config.depthStencilState.maxDepthBounds = 1.0f;  // Optional
	config.depthStencilState.stencilTestEnable = VK_FALSE;
	config.depthStencilState.front = {};  // Optional
	config.depthStencilState.back = {};   // Optional

	config.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	config.dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	config.dynamicState.dynamicStateCount = static_cast<uint32_t>(config.dynamicStates.size());
	config.dynamicState.pDynamicStates = config.dynamicStates.data();
	config.dynamicState.flags = 0;

	config.bindingDescriptions = Model::Vertex::getBindingDescriptions();
	config.attributeDescriptions = Model::Vertex::getAttributeDescriptions();
}

void Pipeline::enableAlphaBlending(PipelineConfig& config)
{
	config.colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	config.colorBlendAttachment.blendEnable = VK_TRUE;
	config.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	config.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	config.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	config.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	config.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	config.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

}
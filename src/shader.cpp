#include "shader.h"

#include <iostream>
#include <fstream>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace VulkanEngine
{

Shader::Shader(Device& device, const std::string& filepath) : m_device{ device }
{
	createShaderModule(filepath);
}

Shader::~Shader()
{
	vkDestroyShaderModule(m_device.getDevice(), m_shaderModule, nullptr);
}

void Shader::createShaderModule(const std::string& filepath)
{
	std::string enginePath = ENGINE_DIR + filepath;
	std::ifstream file{ enginePath, std::ios::ate | std::ios::binary };

	if(!file.is_open())
	{
		throw std::runtime_error("failed to open file: " + filepath);
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> code(fileSize);

	file.seekg(0);
	file.read(code.data(), fileSize);

	file.close();

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if(vkCreateShaderModule(m_device.getDevice(), &createInfo, nullptr, &m_shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("fail to create shader module");
	}
}

}
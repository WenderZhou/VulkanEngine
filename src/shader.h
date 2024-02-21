#pragma once

#include "device.h"

namespace VulkanEngine
{

class Shader
{
public:
	Shader(Device& device, const std::string& filepath);
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&&) = delete;
	Shader& operator=(Shader&&) = delete;

	VkShaderModule getShaderModule() { return m_shaderModule; }

private:
	void createShaderModule(const std::string& filepath);

	Device& m_device;

	VkShaderModule m_shaderModule;
};

}
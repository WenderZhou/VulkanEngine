#pragma once

#include "window.h"
#include "pipeline.h"
#include "device.h"

namespace VulkanEngine
{

class App
{
public:
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 600;

	void run();

private:
	Window window{ WIDTH, HEIGHT, "Vulkan Window" };
	Device device{ window };
	PipelineConfigInfo configInfo{};
	Pipeline pipeline{ 
		device,
		"shaders/basic.vert.spv",
		"shaders/basic.frag.spv",
		Pipeline::defaultPipelineConfigInfo(configInfo, WIDTH, HEIGHT) };
};

}
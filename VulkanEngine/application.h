#pragma once

#include "window.h"
#include "pipeline.h"
#include "device.h"
#include "swapchain.h"

#include <memory>
#include <vector>

namespace VulkanEngine
{

class App
{
public:
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 600;

	App();
	~App();

	App(const App&) = delete;
	App& operator=(const App&) = delete;

	void run();

private:
	void createPipelineLayout();
	void createPipeline();
	void createCommandBuffers();
	void drawFrame();

	Window window{ WIDTH, HEIGHT, "Vulkan Window" };
	Device device{ window };
	Swapchain swapchain{ device, window.getExtent() };

	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkCommandBuffer> commandBuffers;
};

}
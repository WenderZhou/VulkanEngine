#pragma once

#include "window.h"
#include "pipeline.h"
#include "device.h"
#include "swapchain.h"
#include "gameobject.h"

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
	void loadGameObjects();
	void createPipelineLayout();
	void createPipeline();
	void createCommandBuffers();
	void freeCommandBuffers();
	void drawFrame();
	void recreateSwapchain();
	void recordCommandBuffer(int imageIndex);
	void renderGameObjects(VkCommandBuffer commandBuffer);

	Window window{ WIDTH, HEIGHT, "Vulkan Window" };
	Device device{ window };

	std::unique_ptr<Swapchain> swapchain;
	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<GameObject> gameObjects;
};

}
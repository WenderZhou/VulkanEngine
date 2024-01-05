#include "application.h"

#include "renderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace VulkanEngine
{

App::App()
{
	loadGameObjects();
}

App::~App()
{

}

void App::run()
{
	RenderSystem renderSystem{ device, renderer.getSwapchainRenderPass() };

	while (!window.shouldClose())
	{
		glfwPollEvents();
		
		if (VkCommandBuffer commandBuffer = renderer.beginFrame())
		{
			renderer.beginSwapchainRenderPass(commandBuffer);
			renderSystem.renderGameObjects(commandBuffer, gameObjects);
			renderer.endSwapchainRenderPass(commandBuffer);
			renderer.endFrame();
		}
	}

	vkDeviceWaitIdle(device.device());
}

void App::loadGameObjects()
{
	std::vector<Model::Vertex> vertices{
		{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

	std::shared_ptr<Model> model = std::make_shared<Model>(device, vertices);

	GameObject triangle = GameObject::createGameObject();
	triangle.model = model;
	triangle.color = { 0.1f, 0.8f, 0.1f };
	triangle.transform2d.translation.x = 0.2f;
	triangle.transform2d.scale = { 2.0f, 0.5f };
	triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();

	gameObjects.push_back(std::move(triangle));
}

}
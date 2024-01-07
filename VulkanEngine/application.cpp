#include "application.h"

#include "keyboard.h"
#include "camera.h"
#include "renderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <chrono>

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
    Camera camera{};

    // for store the camera state
    auto viewObject = GameObject::createGameObject();
    KeyboardMovementController cameraController{};

    auto lastTime = std::chrono::high_resolution_clock::now();

	while (!window.shouldClose())
	{
		glfwPollEvents();
		
        auto currTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currTime - lastTime).count();
        lastTime = currTime;

        cameraController.moveInPlaneXZ(window.getGLFWWindow(), deltaTime, viewObject);
        camera.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);

        float aspectRatio = renderer.getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.0f), aspectRatio, 0.1f, 100.f);

		if (VkCommandBuffer commandBuffer = renderer.beginFrame())
		{
			renderer.beginSwapchainRenderPass(commandBuffer);
			renderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
			renderer.endSwapchainRenderPass(commandBuffer);
			renderer.endFrame();
		}
	}

	vkDeviceWaitIdle(device.device());
}

void App::loadGameObjects()
{
    std::shared_ptr<Model> model = Model::createModelFromFile(device, "models/smooth_vase.obj");

	GameObject gameObject = GameObject::createGameObject();
	gameObject.model = model;
	gameObject.transform.translation = { 0.0f, 0.0f, 2.5f };
	gameObject.transform.scale = { 3.0f, 3.0f, 3.0f };

	gameObjects.push_back(std::move(gameObject));
}

}
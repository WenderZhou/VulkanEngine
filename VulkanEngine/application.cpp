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

std::unique_ptr<Model> createCubeModel(Device& device, glm::vec3 offset) {
    std::vector<Model::Vertex> vertices{

        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

    };
    for (auto& v : vertices) {
        v.position += offset;
    }
    return std::make_unique<Model>(device, vertices);
}

void App::loadGameObjects()
{
    std::shared_ptr<Model> model = createCubeModel(device, { 0.0f, 0.0f, 0.0f });

	GameObject cube = GameObject::createGameObject();
    cube.model = model;
    cube.transform.translation = { 0.0f, 0.0f, 2.5f };
    cube.transform.scale = { 0.5f, 0.5f, 0.5f };

	gameObjects.push_back(std::move(cube));
}

}
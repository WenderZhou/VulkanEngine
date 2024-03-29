#include "application.h"

#include "keyboard.h"
#include "camera.h"
#include "buffer.h"
#include "systems/gameObjectPass.h"
#include "systems/pointLightPass.h"

#include "image.h"

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
	globalPool.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 * Device::MAX_FRAMES_IN_FLIGHT);
	globalPool.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Device::MAX_FRAMES_IN_FLIGHT);
	globalPool.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
	globalPool.build();

	loadGameObjects();
}

App::~App()
{

}

void App::run()
{
	GameObjectPass gameObjectPass{ device, globalPool };
	PointLightPass pointLightPass{ device, globalPool };
	Camera camera{};

	// for store the camera state
	auto viewObject = GameObject::createGameObject();
	viewObject.transform.translation.z = -2.5f;
	KeyboardMovementController cameraController{};

	UI ui{ window, device, globalPool };

	auto lastTime = std::chrono::high_resolution_clock::now();

	while(!window.shouldClose())
	{
		glfwPollEvents();

		auto currTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(currTime - lastTime).count();
		lastTime = currTime;

		cameraController.moveInPlaneXZ(window.getGLFWWindow(), frameTime, viewObject);
		camera.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);

		float aspectRatio = device.getAspectRatio();
		camera.setPerspectiveProjection(glm::radians(50.0f), aspectRatio, 0.1f, 100.f);

		if(VkCommandBuffer commandBuffer = device.beginFrame())
		{
			int frameIndex = device.getFrameIndex();

			FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, gameObjects };

			gameObjectPass.update(frameInfo);

			device.beginRenderPass(commandBuffer);

			gameObjectPass.render(frameInfo);
			pointLightPass.render(frameInfo);

			ui.render(frameInfo);

			device.endRenderPass(commandBuffer);
			device.endFrame();
		}
	}

	device.waitIdle();
}

void App::loadGameObjects()
{
	std::shared_ptr<Model> pModel = Model::createModelFromFile(device, "models/flat_vase.obj");
	GameObject flatVase = GameObject::createGameObject();
	flatVase.pModel = pModel;
	flatVase.transform.translation = { -0.5f, 0.5f, 0.0f };
	flatVase.transform.scale = { 3.0f, 1.5f, 3.0f };
	gameObjects.emplace(flatVase.getId(), std::move(flatVase));

	pModel = Model::createModelFromFile(device, "models/smooth_vase.obj");
	GameObject smoothVase = GameObject::createGameObject();
	smoothVase.pModel = pModel;
	smoothVase.transform.translation = { 0.5f, 0.5f, 0.0f };
	smoothVase.transform.scale = { 3.0f, 1.5f, 3.0f };
	gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

	pModel = Model::createModelFromFile(device, "models/quad.obj");
	GameObject quad = GameObject::createGameObject();
	quad.pModel = pModel;
	quad.transform.translation = { 0.0f, 0.5f, 0.0f };
	quad.transform.scale = { 3.0f, 1.0f, 3.0f };
	gameObjects.emplace(quad.getId(), std::move(quad));

	std::vector<glm::vec3> lightColors
	{
		{1.f, .1f, .1f},
		{.1f, .1f, 1.f},
		{.1f, 1.f, .1f},
		{1.f, 1.f, .1f},
		{.1f, 1.f, 1.f},
		{1.f, 1.f, 1.f}  //
	};

	for(int i = 0; i < lightColors.size(); i++)
	{
		auto pointLight = GameObject::createPointLight(0.2f, 0.1f, glm::vec3(1.0f, 1.0f, 1.0f));
		pointLight.color = lightColors[i];
		auto rotateLight = glm::rotate(
			glm::mat4(1.f),
			(i * glm::two_pi<float>()) / lightColors.size(),
			{ 0.f, -1.f, 0.f });
		pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
		gameObjects.emplace(pointLight.getId(), std::move(pointLight));
	}
}

}
#include "application.h"

#include "keyboard.h"
#include "camera.h"
#include "buffer.h"
#include "systems/gameObjectSystem.h"
#include "systems/pointLightSystem.h"

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
	globalPool =
		DescriptorPool::Builder(device)
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Device::MAX_FRAMES_IN_FLIGHT)
		.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Device::MAX_FRAMES_IN_FLIGHT)
		.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
		.build();
	loadGameObjects();
}

App::~App()
{

}

void App::run()
{
	std::vector<std::unique_ptr<Buffer>> uboBuffers(Device::MAX_FRAMES_IN_FLIGHT);
	for(int i = 0; i < uboBuffers.size(); i++)
	{
		uboBuffers[i] = std::make_unique<Buffer>(device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uboBuffers[i]->map();
	}

	auto globalSetLayout = DescriptorSetLayout::Builder(device)
		.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.build();

	std::vector<VkDescriptorSet> globalDescriptorSets(Device::MAX_FRAMES_IN_FLIGHT);
	for(int i = 0; i < globalDescriptorSets.size(); i++)
	{
		auto bufferInfo = uboBuffers[i]->descriptorInfo();
		DescriptorWriter(*globalSetLayout, *globalPool).writeBuffer(0, &bufferInfo).build(globalDescriptorSets[i]);
	}

	GameObjectSystem gameObjectSystem{ device, device.getRenderPass(), globalSetLayout->getDescriptorSetLayout() };
	PointLightSystem pointLightSystem{ device, device.getRenderPass(), globalSetLayout->getDescriptorSetLayout() };
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

		VkCommandBuffer commandBuffer = device.beginFrame();
		int frameIndex = device.getFrameIndex();

		FrameInfo frameInfo
		{
			frameIndex,
			frameTime,
			commandBuffer,
			camera,
			globalDescriptorSets[frameIndex],
			gameObjects
		};

		GlobalUbo ubo{};
		ubo.projection = camera.getProjection();
		ubo.view = camera.getView();
		ubo.invView = camera.getInvView();
		pointLightSystem.update(frameInfo, ubo);
		uboBuffers[frameIndex]->writeToBuffer(&ubo);
		uboBuffers[frameIndex]->flush();

		device.beginRenderPass(commandBuffer);

		// order matters
		gameObjectSystem.render(frameInfo);
		pointLightSystem.render(frameInfo);

		ui.render(frameInfo);

		device.endRenderPass(commandBuffer);
		device.endFrame();
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
#include "application.h"

#include "keyboard.h"
#include "camera.h"
#include "renderSystem.h"
#include "buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <chrono>

namespace VulkanEngine
{

struct GlobalUbo
{
	glm::mat4 projectionView{ 1.0f };
	glm::vec4 ambientLightColor{ 1.0f, 1.0f, 1.0f, 0.02f };
	glm::vec3 lightPosition{ -1.0f, -1.0f, -1.0f };
	alignas(16) glm::vec4 lightColor{ 1.0f, 1.0f, 1.0f, 1.0f }; // w as intensity
};

App::App()
{
	globalPool = 
		DescriptorPool::Builder(device)
		.setMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
		.build();
	loadGameObjects();
}

App::~App()
{

}

void App::run()
{
	std::vector<std::unique_ptr<Buffer>> uboBuffers(Swapchain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < uboBuffers.size(); i++)
	{
		uboBuffers[i] = std::make_unique<Buffer>(
			device,
			sizeof(GlobalUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uboBuffers[i]->map();
	}

	auto globalSetLayout = DescriptorSetLayout::Builder(device)
		.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.build();

	std::vector<VkDescriptorSet> globalDescriptorSets(Swapchain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++)
	{
		auto bufferInfo = uboBuffers[i]->descriptorInfo();
		DescriptorWriter(*globalSetLayout, *globalPool)
			.writeBuffer(0, &bufferInfo)
			.build(globalDescriptorSets[i]);
	}

	RenderSystem renderSystem{ device, renderer.getSwapchainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
    Camera camera{};

    // for store the camera state
    auto viewObject = GameObject::createGameObject();
	viewObject.transform.translation.z = -2.5f;
    KeyboardMovementController cameraController{};

    auto lastTime = std::chrono::high_resolution_clock::now();

	while (!window.shouldClose())
	{
		glfwPollEvents();
		
        auto currTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(currTime - lastTime).count();
        lastTime = currTime;

        cameraController.moveInPlaneXZ(window.getGLFWWindow(), frameTime, viewObject);
        camera.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);

        float aspectRatio = renderer.getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.0f), aspectRatio, 0.1f, 100.f);

		if (VkCommandBuffer commandBuffer = renderer.beginFrame())
		{
			int frameIndex = renderer.getFrameIndex();

			FrameInfo frameInfo
			{
				frameIndex,
				frameTime,
				commandBuffer,
				camera,
				globalDescriptorSets[frameIndex]
			};

			GlobalUbo ubo{};
			ubo.projectionView = camera.getProjection() * camera.getView();
			uboBuffers[frameIndex]->writeToBuffer(&ubo);
			uboBuffers[frameIndex]->flush();

			renderer.beginSwapchainRenderPass(commandBuffer);
			renderSystem.renderGameObjects(frameInfo, gameObjects);
			renderer.endSwapchainRenderPass(commandBuffer);
			renderer.endFrame();
		}
	}

	vkDeviceWaitIdle(device.device());
}

void App::loadGameObjects()
{
    std::shared_ptr<Model> model = Model::createModelFromFile(device, "models/flat_vase.obj");
	GameObject flatVase = GameObject::createGameObject();
	flatVase.model = model;
	flatVase.transform.translation = { -0.5f, 0.5f, 0.0f };
	flatVase.transform.scale = { 3.0f, 1.5f, 3.0f };
	gameObjects.push_back(std::move(flatVase));

	model = Model::createModelFromFile(device, "models/smooth_vase.obj");
	GameObject smoothVase = GameObject::createGameObject();
	smoothVase.model = model;
	smoothVase.transform.translation = { 0.5f, 0.5f, 0.0f };
	smoothVase.transform.scale = { 3.0f, 1.5f, 3.0f };
	gameObjects.push_back(std::move(smoothVase));

	model = Model::createModelFromFile(device, "models/quad.obj");
	GameObject quad = GameObject::createGameObject();
	quad.model = model;
	quad.transform.translation = { 0.0f, 0.5f, 0.0f };
	quad.transform.scale = { 3.0f, 1.0f, 3.0f };
	gameObjects.push_back(std::move(quad));
}

}
#pragma once

#include "camera.h"
#include "gameobject.h"

#include <vulkan/vulkan.h>

namespace VulkanEngine
{

#define MAX_LIGHTS 10

struct PointLight
{
	glm::vec4 position{};
	glm::vec4 color{};	// w is intensity
};

struct FrameInfo
{
	int frameIndex;
	float frameTime;
	VkCommandBuffer commandBuffer;
	Camera& camera;
	VkDescriptorSet globalDescriptorSet;
	GameObject::Map& gameObjects;
};

struct GlobalUbo
{
	glm::mat4 projection{ 1.0f };
	glm::mat4 view{ 1.0f };
	glm::vec4 ambientLightColor{ 1.0f, 1.0f, 1.0f, 0.02f };
	PointLight pointLights[MAX_LIGHTS];
	int numLights;
};

}
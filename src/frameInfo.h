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
	GameObject::Map& gameObjects;
};

}
#pragma once

#include "window.h"
#include "device.h"
#include "renderer.h"
#include "gameobject.h"
#include "descriptor.h"

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

	Window window{ WIDTH, HEIGHT, "Vulkan Window" };
	Device device{ window };
	Renderer renderer{ window, device };

	std::unique_ptr<DescriptorPool> globalPool{};
	GameObject::Map gameObjects;
};

}
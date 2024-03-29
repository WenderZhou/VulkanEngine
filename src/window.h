#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace VulkanEngine
{

class Window
{
public:
	Window(int w, int h, std::string name);
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	bool shouldClose() { return glfwWindowShouldClose(glfwWindow); }
	VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }

	bool wasWindowResized() { return framebufferResized; }
	void resetWindowResizedFlag(){ framebufferResized = false; }
	GLFWwindow* getGLFWWindow() const { return glfwWindow; }

	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
	static void framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height);

	int width;
	int height;
	bool framebufferResized = false;

	std::string windowName;

	GLFWwindow* glfwWindow;
};

}
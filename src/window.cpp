#include "window.h"

#include <stdexcept>

namespace VulkanEngine
{

Window::Window(int w, int h, std::string name) :width(w), height(h), windowName(name)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// not generate OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	glfwWindow = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(glfwWindow, this);
	glfwSetFramebufferSizeCallback(glfwWindow, framebufferResizeCallback);
}

Window::~Window()
{
	glfwDestroyWindow(glfwWindow);
	glfwTerminate();
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(instance, glfwWindow, nullptr, surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface");
	}
}

void Window::framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height)
{
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
	window->framebufferResized = true;
	window->width = width;
	window->height = height;
}

}
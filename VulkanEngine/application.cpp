#include "application.h"

namespace VulkanEngine
{

void App::run()
{
	while (!window.shouldClose())
	{
		glfwPollEvents();
	}
}

}
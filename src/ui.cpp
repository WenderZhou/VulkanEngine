#include "ui.h"

namespace VulkanEngine
{

UI::UI(Window &window, Device &device, Renderer &renderer, std::unique_ptr<DescriptorPool> &descriptorPool)
{
    ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplGlfw_InitForVulkan(window.getGLFWWindow(), true);

	ImGui_ImplVulkan_InitInfo info{};
	info.DescriptorPool = descriptorPool->getDescriptorPool();
	info.Device = device.device();
	info.PhysicalDevice = device.getPhysicalDevice();
	info.ImageCount = Swapchain::MAX_FRAMES_IN_FLIGHT;
	info.Instance = device.getInstance();
	info.Queue = device.graphicsQueue();
	info.MinImageCount = 2;

	ImGui_ImplVulkan_Init(&info, renderer.getSwapchainRenderPass());

	ImGui_ImplVulkan_CreateFontsTexture();
}

UI::~UI()
{
    ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void UI::render(VkCommandBuffer commandBuffer)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Text("Hello, world");

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

}

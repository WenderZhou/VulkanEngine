#include "ui.h"

namespace VulkanEngine
{

UI::UI(Window& window, Device& device, std::unique_ptr<DescriptorPool>& descriptorPool)
{
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplGlfw_InitForVulkan(window.getGLFWWindow(), true);

	ImGui_ImplVulkan_InitInfo info{};
	info.DescriptorPool = descriptorPool->getDescriptorPool();
	info.Device = device.getDevice();
	info.PhysicalDevice = device.getPhysicalDevice();
	info.ImageCount = Device::MAX_FRAMES_IN_FLIGHT;
	info.Instance = device.getInstance();
	info.Queue = device.getGraphicsQueue();
	info.MinImageCount = 2;

	ImGui_ImplVulkan_Init(&info, device.getRenderPass());

	ImGui_ImplVulkan_CreateFontsTexture();
}

UI::~UI()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void UI::render(FrameInfo& frameInfo)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Scene");
	ImGui::Text("Scene");
	ImGui::End();

	ImGui::Begin("Inspector");
	ImGui::Text("Inspector");
	ImGui::End();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
}

}

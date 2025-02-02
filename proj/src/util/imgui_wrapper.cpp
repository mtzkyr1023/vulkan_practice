
#include "imgui_wrapper.h"
#include "../imgui/imgui.h"
#include "../imgui/imconfig.h"
#include "../imgui/backends/imgui_impl_vulkan.h"
#include "../imgui/backends/imgui_impl_win32.h"
#include "../render_engine.h"


void ImGuiWrapper::setup(RenderEngine* engine, HWND hwnd)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hwnd);

	{
		std::vector<vk::DescriptorPoolSize> poolSizes = {
			{vk::DescriptorType::eSampler,1000},
			{vk::DescriptorType::eCombinedImageSampler,1000},
			{vk::DescriptorType::eSampledImage,1000},
			{vk::DescriptorType::eStorageImage,1000},
			{vk::DescriptorType::eUniformTexelBuffer,1000},
			{vk::DescriptorType::eStorageTexelBuffer,1000},
			{vk::DescriptorType::eUniformBuffer,1000},
			{vk::DescriptorType::eStorageBuffer,1000},
			{vk::DescriptorType::eUniformBufferDynamic,1000},
			{vk::DescriptorType::eStorageBufferDynamic,1000},
			{vk::DescriptorType::eInputAttachment,1000}
		};
		vk::DescriptorPoolCreateInfo poolCreateInfo = vk::DescriptorPoolCreateInfo()
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
			.setMaxSets(1000)
			.setPoolSizes(poolSizes);

		pool_ = engine->device().createDescriptorPool(poolCreateInfo);
	}

	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = engine->instance();
	initInfo.PhysicalDevice = engine->physicalDevice();
	initInfo.Device = engine->device();
	initInfo.QueueFamily = engine->graphicsQueueFamilyIndex();
	initInfo.Queue = engine->graphicsQueue();
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = pool_;
	initInfo.RenderPass = engine->renderPass();
	initInfo.Subpass = 0;
	initInfo.MinImageCount = engine->swapchainImageCount();
	initInfo.ImageCount = engine->swapchainImageCount();
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = nullptr;

	ImGui_ImplVulkan_Init(&initInfo);
}

void ImGuiWrapper::cleanup(RenderEngine* engine)
{
	ImGui_ImplVulkan_Shutdown();
	engine->device().destroyDescriptorPool(pool_);
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


void ImGuiWrapper::prepare()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiWrapper::render(RenderEngine* engine, const vk::CommandBuffer& cb)
{
	ImGui::Render();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb);
}

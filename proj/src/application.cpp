#include "application.h"
#include "render_engine.h"
#include "util/timer.h"
#include "defines.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "util/input.h"

Application::Application() {

}

Application::~Application() {

}

void Application::initialize(RenderEngine* engine, HWND hwnd) {
	try
	{
		imgui_.setup(engine, hwnd);
		deferredPass_.setup(engine);
		simplePipeline_.initialize(engine, &deferredPass_, nullptr);
		fbPipeline_.initialize(engine, nullptr, &deferredPass_);

		vk::SamplerCreateInfo samplerCreateInfo = vk::SamplerCreateInfo()
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setAnisotropyEnable(true)
			.setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
			.setCompareEnable(false)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMagFilter(vk::Filter::eLinear)
			.setMaxAnisotropy(16.0f)
			.setMinFilter(vk::Filter::eLinear)
			.setMaxLod(0)
			.setMipLodBias(0)
			.setMinLod(0)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear);

		sampler_ = engine->device().createSampler(samplerCreateInfo);

		depthBuffer_ = ImGui_ImplVulkan_AddTexture(sampler_, deferredPass_.imageView((uint32_t)DeferredPass::eTemporary), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		Input::Instance().Initialize(hwnd);
	}
	catch (std::exception& e) {
		wchar_t buf[8192];
		size_t size;
		mbstowcs_s(&size, buf, e.what(), 8192);
		OutputDebugString(buf);
		MessageBox(nullptr, buf, L"Error", MB_OK);
	}
}

void Application::cleanup(RenderEngine* engine) {
	engine->device().waitIdle();
	engine->graphicsQueue().waitIdle();

	engine->device().destroySampler(sampler_);

	simplePipeline_.cleanup(engine);
	fbPipeline_.cleanup(engine);
	deferredPass_.cleanup(engine);
	imgui_.cleanup(engine);
}

bool Application::render(RenderEngine* engine) {
	try
	{
		uint32_t currentFrameIndex = engine->acquireNextImage();

		update(engine, currentFrameIndex);

		vk::CommandBuffer cb = engine->commandBuffer(currentFrameIndex);

		cb.begin(vk::CommandBufferBeginInfo());

		simplePipeline_.render(engine, &deferredPass_, currentFrameIndex);


		std::array<vk::ClearValue, 1> clearValues = {
		vk::ClearValue()
		};
		vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
			.setRenderPass(engine->renderPass())
			.setFramebuffer(engine->framebuffer(currentFrameIndex))
			.setRenderArea(vk::Rect2D({ 0, 0 }, { kScreenWidth, kScreenHeight }))
			.setClearValues(clearValues);

		cb.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		fbPipeline_.render(engine, nullptr, currentFrameIndex);


		imgui_.render(engine, cb);

		cb.endRenderPass();

		cb.end();

		std::vector<vk::PipelineStageFlags> waitStageMasks = {
			vk::PipelineStageFlagBits::eBottomOfPipe
		};

		std::vector<vk::Semaphore> waitSemaphores = {
			engine->presentCompletedSemaphore(),
		};

		vk::Fence submitFence = engine->device().createFence(vk::FenceCreateInfo());

		std::vector<vk::Semaphore> signalSemaphores = {
			simplePipeline_.renderCompletedSemaphore(currentFrameIndex)
		};

		vk::SubmitInfo submitInfo = vk::SubmitInfo()
			.setCommandBufferCount(1)
			.setPCommandBuffers(&cb)
			.setWaitSemaphoreCount(1)
			.setWaitSemaphores(waitSemaphores)
			.setWaitDstStageMask(waitStageMasks)
			.setSignalSemaphoreCount(1)
			.setSignalSemaphores(signalSemaphores);

		engine->graphicsQueue().submit(submitInfo, submitFence);

		vk::Fence waitFences[1] = {
			submitFence,
		};

		engine->device().waitForFences(waitFences, vk::True, kTimeOut);

		engine->present(signalSemaphores);

		engine->device().destroyFence(submitFence);

		if (Input::Instance().Trigger(DIK_ESCAPE)) return false;

		Timer::instance().update();
		Input::Instance().Updata();
	}
	catch (std::exception& e) {
		wchar_t buf[8192];
		size_t size;
		mbstowcs_s(&size, buf, e.what(), 8192);
		OutputDebugString(buf);
	}

	return true;
}

void Application::update(RenderEngine* engine, uint32_t currentFrameIndex)
{
	imgui_.prepare();

	ImGui::Begin("Hello world!");

	ImGui::Text("FPS:%1f, DeltaTime:%1f", Timer::instance().fps(), Timer::instance().deltaTime());

	simplePipeline_.update(engine, currentFrameIndex);

	ImGui::End();
}

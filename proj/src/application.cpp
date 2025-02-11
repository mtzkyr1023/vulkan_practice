#include "application.h"
#include "render_engine.h"
#include "util/timer.h"
#include "defines.h"
#include "imgui/imgui.h"

Application::Application() {

}

Application::~Application() {

}

void Application::initialize(RenderEngine* engine, HWND hwnd) {
	imgui_.setup(engine, hwnd);
	deferredPass_.setup(engine);
	simplePipeline_.initialize(engine, &deferredPass_);
}

void Application::cleanup(RenderEngine* engine) {
	engine->device().waitIdle();
	engine->graphicsQueue().waitIdle();
	simplePipeline_.cleanup(engine);
	deferredPass_.cleanup(engine);
	imgui_.cleanup(engine);
}

void Application::render(RenderEngine* engine) {
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
		.setClearValueCount(1)
		.setPClearValues(clearValues.data());

	cb.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

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

	Timer::instance().update();
}

void Application::update(RenderEngine* engine, uint32_t currentFrameIndex)
{
	imgui_.prepare();

	ImGui::Begin("Hello world!");

	ImGui::End();


	simplePipeline_.update(engine, currentFrameIndex);
}

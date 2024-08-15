#include "application.h"
#include "render_engine.h"
#include "defines.h"

Application::Application() {

}

Application::~Application() {

}

void Application::initialize(RenderEngine* engine) {
	simplePipeline_.initialize(engine);
}

void Application::cleanup(RenderEngine* engine) {
	engine->device().waitIdle();
	engine->graphicsQueue().waitIdle();
	simplePipeline_.cleanup(engine);
}

void Application::render(RenderEngine* engine) {
	uint32_t currentFrameIndex = engine->acquireNextImage();

	simplePipeline_.render(engine, currentFrameIndex);

	vk::CommandBuffer cb = simplePipeline_.commandBuffer(0);

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
}
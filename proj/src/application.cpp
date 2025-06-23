
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

#include "application.h"
#include "render_engine.h"
#include "util/timer.h"
#include "defines.h"
#include "util/input.h"

Application::Application() {

}

Application::~Application() {

}

void Application::initialize(RenderEngine* engine, HWND hwnd) {
	try
	{
		testscene_.initialize(engine);

		cameraViewProjBuffer_.setupUniformBuffer(engine, sizeof(glm::mat4) * 4, engine->swapchainImageCount());
		shadowViewProjBuffer_.setupUniformBuffer(engine, sizeof(glm::mat4) * 4, engine->swapchainImageCount());
		cameraInvViewProjBuffer_.setupUniformBuffer(engine, sizeof(glm::mat4) * 4, engine->swapchainImageCount());
		sceneInfoBuffer_.setupUniformBuffer(engine, sizeof(glm::mat4) * 4, engine->swapchainImageCount());
		skyboxInfoBuffer_.setupUniformBuffer(engine, sizeof(glm::mat4) * 4, engine->swapchainImageCount());
		vsmWeightsBuffer_.setupUniformBuffer(engine, sizeof(glm::mat4) * 4, engine->swapchainImageCount());
		shCoeffBufffer_.setupStorageBuffer(engine, sizeof(glm::vec3) * 9, 1, nullptr);

		albedoBuffer_.setupRenderTarget2d(
			engine,
			(uint32_t)kScreenWidth,
			(uint32_t)kScreenHeight,
			vk::Format::eR16G16B16A16Sfloat);
		normalDepthBuffer_.setupRenderTarget2d(
			engine,
			(uint32_t)kScreenWidth,
			(uint32_t)kScreenHeight,
			vk::Format::eR16G16B16A16Sfloat);
		roughMetalVelocityBuffer_.setupRenderTarget2d(
			engine,
			(uint32_t)kScreenWidth,
			(uint32_t)kScreenHeight,
			vk::Format::eR16G16B16A16Sfloat);
		depthStencilBuffer_.setupDepthStencilBuffer(
			engine,
			(uint32_t)kScreenWidth,
			(uint32_t)kScreenHeight);
		compositionBuffer_.setupRenderTarget2d(
			engine,
			(uint32_t)kScreenWidth,
			(uint32_t)kScreenHeight,
			vk::Format::eR16G16B16A16Sfloat);
		deferredResultBuffer_.setupRenderTarget2d(
			engine,
			(uint32_t)kScreenWidth,
			(uint32_t)kScreenHeight,
			vk::Format::eR16G16B16A16Sfloat);
		shadowMap_.setupRenderTarget2d(
			engine,
			(uint32_t)kShadowMapWidth,
			(uint32_t)kShadowMapHeight,
			vk::Format::eR32G32Sfloat,
			true);
		shadowDepthBuffer_.setupDepthStencilBuffer(
			engine,
			(uint32_t)kShadowMapWidth,
			(uint32_t)kShadowMapHeight);
		shadowBlurXBuffer_.setupRenderTarget2d(
			engine,
			(uint32_t)kShadowMapWidth,
			(uint32_t)kShadowMapHeight,
			vk::Format::eR32G32Sfloat,
			true);
		shadowBlurYBuffer_.setupRenderTarget2d(
			engine,
			(uint32_t)kShadowMapWidth,
			(uint32_t)kShadowMapHeight,
			vk::Format::eR32G32Sfloat,
			true);
		shadowResultBuffer_.setupRenderTarget2d(
			engine,
			(uint32_t)kShadowMapWidth,
			(uint32_t)kShadowMapHeight,
			vk::Format::eR32G32Sfloat);

		//sponzaModel_.loadMesh(engine, "models/sponza/gltf/", "sponza.gltf");
		sponzaModel_.loadMesh(engine, "models/ABeautifulGame/gltf/", "ABeautifulGame.gltf");
		sphereModel_.loadMesh(engine, "models/", "sphere.gltf");

		cubemapTexture_.setupResourceCubemap(engine, "textures/cubemaps/industrial_sunset_puresky_1k.hdr");

		imgui_.setup(engine, hwnd);
		shadowPass_.setup(
			engine,	
			{
				&shadowMap_,
				&shadowDepthBuffer_,
			}
		);
		deferredPass_.setup(
			engine,
			{
				&deferredResultBuffer_,
				&albedoBuffer_,
				&normalDepthBuffer_,
				&roughMetalVelocityBuffer_,
				&compositionBuffer_,
				&depthStencilBuffer_,
			}
			);

		shadowPipeline_.initialize(
			engine,
			&shadowPass_,
			{
				&shadowMap_,
				&shadowDepthBuffer_,
			},
			{
				&shadowViewProjBuffer_,
				&cameraViewProjBuffer_,
			},
			{
				&sponzaModel_,
			});
			shadowBlurPipeline_.initialize(
				engine,
				nullptr,
				{
					&shadowMap_,
					&shadowBlurXBuffer_,
					&shadowBlurYBuffer_,
				},
				{
					&vsmWeightsBuffer_,
				},
				{

				}
			);
		simplePipeline_.initialize(
			engine,
			&deferredPass_,
			{
				&albedoBuffer_,
				&normalDepthBuffer_,
				&roughMetalVelocityBuffer_,
				&depthStencilBuffer_,
				&shadowBlurYBuffer_,
				&compositionBuffer_,
				&deferredResultBuffer_,
				&cubemapTexture_
			},
			{
				&cameraViewProjBuffer_,
				&shadowViewProjBuffer_,
				&cameraInvViewProjBuffer_,
				&sceneInfoBuffer_,
				&skyboxInfoBuffer_,
				&shCoeffBufffer_,
			},
			{
				&sponzaModel_,
				&sphereModel_,
			});
		fbPipeline_.initialize(
			engine,
			nullptr,
			{
				&compositionBuffer_,
			},
			{

			},
			{

			});

		PrtPipeline prtPipeline(cubemapTexture_.width());
		prtPipeline.initialize(
			engine,
			nullptr,
			{
				&cubemapTexture_,
			},
			{
				&shCoeffBufffer_,
			},
			{

			}
			);

		prtPipeline.render(engine, nullptr, 0);

		prtPipeline.cleanup(engine);

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
				.setMaxLod(FLT_MAX)
				.setMipLodBias(0)
				.setMinLod(0)
				.setMipmapMode(vk::SamplerMipmapMode::eLinear);

			sampler_ = engine->device().createSampler(samplerCreateInfo);

			shadowMapDebug_ = ImGui_ImplVulkan_AddTexture(sampler_, shadowBlurYBuffer_.view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		Input::Instance().Initialize(hwnd);

		{
			struct Weights
			{
				float weight;
				glm::vec3 padding;
			};
			float sigma = 0.5f;
			Weights weights[kBlurSize];
			float total = 0.0f;
			float d = sigma * sigma;
			for (int i = 0; i < kBlurSize; i++)
			{
				float pos = 1.0f + 2.0f * (float)(i - kBlurSize / 2);
				weights[i].weight = glm::exp(-0.5f * pos * pos / d);
				total += weights[i].weight;
			}

			for (int i = 0; i < kBlurSize; i++)
			{
				weights[i].weight /= total;
			}

			vsmWeightsBuffer_.update(engine, 0, sizeof(glm::vec4) * 8, weights);
		}
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

	cameraViewProjBuffer_.release(engine);
	shadowViewProjBuffer_.release(engine);
	cameraInvViewProjBuffer_.release(engine);
	sceneInfoBuffer_.release(engine);
	skyboxInfoBuffer_.release(engine);
	vsmWeightsBuffer_.release(engine);
	shCoeffBufffer_.release(engine);

	albedoBuffer_.release(engine);
	normalDepthBuffer_.release(engine);
	roughMetalVelocityBuffer_.release(engine);
	depthStencilBuffer_.release(engine);
	compositionBuffer_.release(engine);
	deferredResultBuffer_.release(engine);
	shadowMap_.release(engine);
	shadowDepthBuffer_.release(engine);
	shadowBlurXBuffer_.release(engine);
	shadowBlurYBuffer_.release(engine);
	shadowResultBuffer_.release(engine);

	cubemapTexture_.release(engine);
	sponzaModel_.release(engine);
	sphereModel_.release(engine);

	testscene_.cleanup(engine);
	shadowPipeline_.cleanup(engine);
	shadowBlurPipeline_.cleanup(engine);
	simplePipeline_.cleanup(engine);
	fbPipeline_.cleanup(engine);
	shadowPass_.cleanup(engine);
	deferredPass_.cleanup(engine);
	imgui_.cleanup(engine);

	engine->device().destroySampler(sampler_);
}

bool Application::render(RenderEngine* engine) {
	try
	{
		uint32_t currentFrameIndex = engine->acquireNextImage();

		update(engine, currentFrameIndex);

		vk::CommandBuffer cb = engine->commandBuffer(currentFrameIndex);

		cb.begin(vk::CommandBufferBeginInfo());

		shadowPipeline_.render(engine, &shadowPass_, currentFrameIndex);

		shadowBlurPipeline_.render(engine, nullptr, currentFrameIndex);

		simplePipeline_.render(engine, &deferredPass_, currentFrameIndex);


		std::array<vk::ClearValue, 1> clearValues = {
		vk::ClearValue()
		};
		vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
			.setRenderPass(engine->renderPass())
			.setFramebuffer(engine->framebuffer(currentFrameIndex))
			.setRenderArea(vk::Rect2D({ 0, 0 }, { kFramebufferWidth, kFramebufferHeight }))
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
	ImGui::Text("CamPos:(%f,%f,%f)",
		testscene_.camera().transform().position().x,
		testscene_.camera().transform().position().y,
		testscene_.camera().transform().position().z);

	ImGui::Image((ImTextureID)shadowMapDebug_, ImVec2(512, 512));

	shadowPipeline_.update(engine, currentFrameIndex);
	simplePipeline_.update(engine, currentFrameIndex);

	ImGui::End();

	testscene_.update(Timer::instance().deltaTime());

	testscene_.shadowCaster().range() = glm::length(sponzaModel_.aabbMax() - sponzaModel_.center());

	struct ViewProj
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 world;
	};

	struct SceneInfo
	{
		glm::vec4 lightVec;
		glm::vec4 cameraPos;
		glm::vec4 sceneInfo;
	};

	ViewProj cameraVp;
	ViewProj shadowVp;
	ViewProj camerainvVp;
	SceneInfo sceneInfo;
	ViewProj skyboxVp;

	float scale = 500.0f;
	float range = glm::length(sponzaModel_.aabbMax() - sponzaModel_.aabbMin()) * scale;
	glm::vec3 aabb = (sponzaModel_.aabbMax() + sponzaModel_.aabbMin()) / 2.0f;
	testscene_.shadowCaster().range() = range;
	testscene_.shadowCaster().width() = (float)kShadowMapWidth / range * 100.0f;
	testscene_.shadowCaster().height() = (float)kShadowMapHeight / range * 100.0f;
	testscene_.shadowCaster().transform().position() = aabb;

	cameraVp.view = testscene_.camera().viewMatrix();
	cameraVp.proj = testscene_.camera().projMatrix();

	static float x;

	if (Input::Instance().Push(DIK_Z))
		x += Timer::instance().deltaTime();

	cameraVp.world = glm::rotate(glm::identity<glm::mat4>(), x, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::identity<glm::mat4>(), glm::vec3(scale, scale, scale));

	shadowVp.view = testscene_.shadowCaster().viewMatrix();
	shadowVp.proj = testscene_.shadowCaster().projMatrix();
	shadowVp.world = glm::scale(glm::identity<glm::mat4>(), glm::vec3(scale, scale, scale));

	camerainvVp.view = glm::inverse(cameraVp.view);
	camerainvVp.proj = glm::inverse(cameraVp.proj);

	sceneInfo.lightVec = glm::vec4(testscene_.shadowCaster().transform().forward() * -1.0f, 0.0f);
	sceneInfo.cameraPos = glm::vec4(testscene_.camera().transform().position(), 0.0f);
	sceneInfo.sceneInfo = glm::vec4((float)kScreenWidth, (float)kScreenHeight, testscene_.camera().nearZ(), testscene_.camera().farZ());

	skyboxVp.world = glm::scale(glm::identity<glm::mat4>(), glm::vec3(10000.0f));
	skyboxVp.view = testscene_.camera().viewMatrix();
	skyboxVp.proj = testscene_.camera().projMatrix();

	cameraViewProjBuffer_.update(engine, currentFrameIndex * sizeof(glm::mat4) * 4, sizeof(ViewProj), &cameraVp);
	shadowViewProjBuffer_.update(engine, currentFrameIndex * sizeof(glm::mat4) * 4, sizeof(ViewProj), &shadowVp);
	cameraInvViewProjBuffer_.update(engine, currentFrameIndex * sizeof(glm::mat4) * 4, sizeof(ViewProj), &camerainvVp);
	sceneInfoBuffer_.update(engine, currentFrameIndex * sizeof(glm::mat4) * 4, sizeof(SceneInfo), &sceneInfo);
	skyboxInfoBuffer_.update(engine, currentFrameIndex * sizeof(glm::mat4) * 4, sizeof(ViewProj), &skyboxVp);
}

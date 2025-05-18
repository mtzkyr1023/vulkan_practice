#ifndef _APPLICATION_H_
#define _APPLICATION_H_


#include "render_pipeine/impl/shadow_pipeline.h"
#include "render_pipeine/impl/shadow_blur_pipeline.h"
#include "render_pipeine/impl/simple_pipeline.h"
#include "render_pipeine/impl/fb_pipeline.h"
#include "render_pass/impl/deferred_pass.h"
#include "render_pass/impl/shadow_pass.h"
#include "util/imgui_wrapper.h"
#include "gameobject/test_scene.h"
#include "resource/buffer.h"
#include "resource/texture.h"
#include "util/mesh.h"

class Application {
public:
	Application();
	~Application();

	void initialize(class RenderEngine* engine, HWND hwnd);
	void cleanup(class RenderEngine* engine);

	bool render(class RenderEngine* engine);


private:
	void update(class RenderEngine* engine, uint32_t currentFrameIndex);

private:
	ShadowPipeline shadowPipeline_;
	ShadowBlurPipeline shadowBlurPipeline_;
	SimplePipeline simplePipeline_;
	ShadowPass shadowPass_;
	DeferredPass deferredPass_;
	FbPipeline fbPipeline_;
	ImGuiWrapper imgui_;
	TestScene testscene_;

	Buffer cameraViewProjBuffer_;
	Buffer shadowViewProjBuffer_;
	Buffer cameraInvViewProjBuffer_;
	Buffer sceneInfoBuffer_;
	Buffer skyboxInfoBuffer_;
	Buffer vsmWeightsBuffer_;

	Texture albedoBuffer_;
	Texture normalDepthBuffer_;
	Texture roughMetalVelocityBuffer_;
	Texture depthStencilBuffer_;
	Texture compositionBuffer_;
	Texture shadowMap_;
	Texture shadowDepthBuffer_;
	Texture shadowBlurXBuffer_;
	Texture shadowBlurYBuffer_;
	Texture deferredResultBuffer_;
	Texture shadowResultBuffer_;

	Texture cubemapTexture_;

	Mesh sponzaModel_;
	Mesh sphereModel_;

	VkDescriptorSet shadowMapDebug_;
	vk::Sampler sampler_;
};

#endif
#ifndef _APPLICATION_H_
#define _APPLICATION_H_


#include "render_pipeine/impl/shadow_pipeline.h"
#include "render_pipeine/impl/simple_pipeline.h"
#include "render_pipeine/impl/fb_pipeline.h"
#include "render_pass/impl/deferred_pass.h"
#include "render_pass/impl/shadow_pass.h"
#include "util/imgui_wrapper.h"
#include "gameobject/test_scene.h"

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
	SimplePipeline simplePipeline_;
	ShadowPass shadowPass_;
	DeferredPass deferredPass_;
	FbPipeline fbPipeline_;
	ImGuiWrapper imgui_;
	vk::Sampler sampler_;
	VkDescriptorSet depthBuffer_;
	TestScene testscene_;
};

#endif
#ifndef _APPLICATION_H_
#define _APPLICATION_H_


#include "render_pipeine/impl/simple_pipeline.h"
#include "render_pass/impl/deferred_pass.h"
#include "util/imgui_wrapper.h"

class Application {
public:
	Application();
	~Application();

	void initialize(class RenderEngine* engine, HWND hwnd);
	void cleanup(class RenderEngine* engine);

	void render(class RenderEngine* engine);


private:
	void update(class RenderEngine* engine, uint32_t currentFrameIndex);

private:
	SimplePipeline simplePipeline_;
	DeferredPass deferredPass_;
	ImGuiWrapper imgui_;
};

#endif
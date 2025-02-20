#include "render_pass.h"
#include "../render_engine.h"


RenderPass::RenderPass()
{

}

RenderPass::~RenderPass()
{

}

void RenderPass::setup(RenderEngine* engine)
{
	setupInternal(engine);
}

void RenderPass::cleanup(RenderEngine* engine)
{
	for (auto& image : images_)
	{
		engine->device().destroyImage(image);
	}
	for (auto& view : imageViews_)
	{
		engine->device().destroyImageView(view);
	}

	for (auto& memory : memories_)
	{
		memory.free(engine->device());
	}

	engine->device().destroyRenderPass(renderPass_);
	engine->device().destroyFramebuffer(framebuffer_);
}


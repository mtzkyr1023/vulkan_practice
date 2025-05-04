#include "render_pass.h"
#include "../render_engine.h"


RenderPass::RenderPass()
{

}

RenderPass::~RenderPass()
{

}

void RenderPass::setup(RenderEngine* engine, const std::vector<class Texture*>& resources)
{
	setupInternal(engine, resources);
}

void RenderPass::cleanup(RenderEngine* engine)
{
	engine->device().destroyRenderPass(renderPass_);
	engine->device().destroyFramebuffer(framebuffer_);
}


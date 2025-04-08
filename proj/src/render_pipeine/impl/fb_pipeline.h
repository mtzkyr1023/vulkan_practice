#ifndef _FB_PIPELINE_H_
#define _FB_PIPELINE_H_

#include "../render_pipeline.h"




class FbPipeline : public RenderPipeline
{
public:
	FbPipeline();
	~FbPipeline();

	virtual void initialize(class RenderEngine* engine, class RenderPass* pass, class RenderPass* prePass, class Scene* scene) override;
	virtual void cleanup(class RenderEngine* engine) override;

	virtual void render(class RenderEngine* engine, class RenderPass* pass, uint32_t currentImageIndex) override;
	virtual void update(class RenderEngine* engine, uint32_t currentImageIndex) override;

protected:

	vk::Sampler sampler_;

};

#endif
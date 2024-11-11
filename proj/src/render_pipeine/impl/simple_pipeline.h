#ifndef _SIMPLE_PIPELINE_H_
#define _SIMPLE_PIPELINE_H_


#include <vulkan/vulkan.h>
#include "../render_pipeline.h"

class SimplePipeline : public RenderPipeline {
public:
	SimplePipeline();
	~SimplePipeline();

	virtual void initialize(class RenderEngine* engine) override;
	virtual void cleanup(class RenderEngine* engine) override;

	virtual void render(class RenderEngine* engine, uint32_t currentImageIndex);

protected:
	std::vector<vk::Framebuffer> framebuffers_;
	std::vector<BufferSp> viewProjBuffer_;
};

#endif
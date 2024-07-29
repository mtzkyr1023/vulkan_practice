#ifndef _RENDER_PIPELINE_H_
#define _RENDER_PIPELINE_H_

#include <vulkan/vulkan.hpp>

class RenderPipeline {
public:
	virtual ~RenderPipeline() {}

	virtual void initialize(class RenderEngine* engine) = 0;
	virtual void render(vk::CommandBuffer commandBuffer) = 0;

	virtual void cleanup(class RenderEngine* engine);

protected:
	vk::ShaderModule createShaderModule(class RenderEngine* engine, const std::string& filename);

protected:
	vk::RenderPass renderPass_;
	vk::Pipeline pipeline_;
};

#endif
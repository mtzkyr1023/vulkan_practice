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

	virtual void render(class RenderEngine* engine, uint32_t currentImageIndex) override;
	virtual void update(class RenderEngine* engine, uint32_t currentImageIndex) override;

protected:
	std::vector<vk::Buffer> viewProjBuffer_;
	Memory ubMemory;
	std::vector<vk::DescriptorSetLayout> layouts_;
	std::vector<vk::DescriptorSet> sets_;

	uint8_t* mappedViewProjMemory_ = nullptr;
};

#endif
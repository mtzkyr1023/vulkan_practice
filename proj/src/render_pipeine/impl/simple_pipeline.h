#ifndef _SIMPLE_PIPELINE_H_
#define _SIMPLE_PIPELINE_H_


#include <vulkan/vulkan.h>
#include "../render_pipeline.h"

class SimplePipeline : public RenderPipeline {
public:
	enum ESubpassType : uint32_t
	{
		eDepthPrePass = 0,
		eGBuffer,
		eComposition,
		eTransparent,

		eNum,
	};

	SimplePipeline();
	~SimplePipeline();

	virtual void initialize(class RenderEngine* engine, class RenderPass* pass) override;
	virtual void cleanup(class RenderEngine* engine) override;

	virtual void render(class RenderEngine* engine, class RenderPass* pass, uint32_t currentImageIndex) override;
	virtual void update(class RenderEngine* engine, uint32_t currentImageIndex) override;

protected:
	std::vector<vk::Buffer> viewProjBuffer_;
	Memory ubMemory_;
	std::vector<vk::DescriptorSet> sets_;
	Memory vertexMemory_;
	Memory indexMemory_;
	Memory tempMemory_;

	uint8_t* mappedViewProjMemory_ = nullptr;
};

#endif
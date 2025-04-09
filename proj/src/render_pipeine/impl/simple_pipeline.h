#ifndef _SIMPLE_PIPELINE_H_
#define _SIMPLE_PIPELINE_H_


#include <vulkan/vulkan.h>
#include "../render_pipeline.h"
#include "../../util/mesh.h"

#include "../../gameobject/camera.h"

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

	virtual void initialize(class RenderEngine* engine, class RenderPass* pass, class RenderPass* prePass, class Scene* scene) override;
	virtual void cleanup(class RenderEngine* engine) override;

	virtual void render(class RenderEngine* engine, class RenderPass* pass, class Scene* scene, uint32_t currentImageIndex) override;
	virtual void update(class RenderEngine* engine, class Scene* scene, uint32_t currentImageIndex) override;

protected:
	std::vector<vk::Buffer> viewProjBuffer_;
	std::vector<vk::Buffer> invViewProjBuffer_;
	std::vector<vk::Buffer> sceneInfoBuffer_;
	std::vector<vk::Buffer> shadowBuffer_;
	Memory ubMemory_;

	vk::Sampler wrapSampler_;
	vk::Sampler clampSampler_;

	uint8_t* mappedViewProjMemory_ = nullptr;
};

#endif
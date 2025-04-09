#ifndef _SHADOW_PIPELINE_H_
#define _SHADOW_PIPELINE_H_

#include "../render_pipeline.h"
#include "../../gameobject/camera.h"

#include "../../util/mesh.h"


class ShadowPipeline : public RenderPipeline
{
public:

	enum ESubpassType : uint32_t
	{
		eRaw = 0,
		eBlurX,
		eBlurY,

		eNum,
	};

	ShadowPipeline();
	~ShadowPipeline();

	virtual void initialize(class RenderEngine* engine, class RenderPass* pass, class RenderPass* prePass, class Scene* scene) override;
	virtual void cleanup(class RenderEngine* engine) override;

	virtual void render(class RenderEngine* engine, class RenderPass* pass, class Scene* scene, uint32_t currentImageIndex) override;
	virtual void update(class RenderEngine* engine, class Scene* scene, uint32_t currentImagendex) override;

protected:
	std::vector<vk::Buffer> viewProjBuffer_;
	std::vector<vk::Buffer> cameraBuffer_;

	Memory ubMemory_;

	vk::Sampler sampler_;

	uint8_t* mappedViewProjMemory_ = nullptr;
};



#endif
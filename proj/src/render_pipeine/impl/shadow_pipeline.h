﻿#ifndef _SHADOW_PIPELINE_H_
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

	enum EBufferType : uint32_t
	{
		eShadowViewProj = 0,
		eCameraViewPoj,
	};

	ShadowPipeline();
	~ShadowPipeline();

	virtual void initialize(
		class RenderEngine* engine,
		class RenderPass* pass,
		const std::vector<class Texture*>& textures,
		const std::vector<class Buffer*>& buffers,
		const std::vector<class Mesh*>& meshes) override;
	virtual void cleanup(class RenderEngine* engine) override;

	virtual void render(class RenderEngine* engine, class RenderPass* pass, uint32_t currentImageIndex) override;
	virtual void update(class RenderEngine* engine, uint32_t currentImagendex) override;

protected:
	vk::Sampler sampler_;

	uint8_t* mappedViewProjMemory_ = nullptr;
};



#endif
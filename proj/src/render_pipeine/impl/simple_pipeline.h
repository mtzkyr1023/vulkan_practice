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
		eGBuffer,
		eComposition,

		eSkybox,

		eTransparent,

		eNum,
	};

	enum ETextureType : uint32_t
	{
		eAlbedo = 0,
		eNormalDepth,
		eRoughMetalVelocity,
		eDepthStencil,
		eShadowMap,
		eDeferredResult,
		eResult,
		eCubeMap,
	};

	enum EBufferType : uint32_t
	{
		eCameraViewProj = 0,
		eShadowViewProj,
		eCameraInv,
		eSceneInfo,
		eSkyboxInfo,
		eShCoeff,
	};

	enum EMeshType : uint32_t
	{
		eBackground,
		eSphere,
	};

	SimplePipeline();
	~SimplePipeline();

	virtual void initialize(
		class RenderEngine* engine,
		class RenderPass* pass,
		const std::vector<class Texture*>& textures,
		const std::vector<class Buffer*>& buffers,
		const std::vector<class Mesh*>& meshes) override;
	virtual void cleanup(class RenderEngine* engine) override;

	virtual void render(class RenderEngine* engine, class RenderPass* pass, uint32_t currentImageIndex) override;
	virtual void update(class RenderEngine* engine, uint32_t currentImageIndex) override;

protected:

	vk::Sampler wrapSampler_;
	vk::Sampler clampSampler_;
};

#endif
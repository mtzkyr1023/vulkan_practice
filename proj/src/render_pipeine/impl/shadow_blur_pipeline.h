#ifndef _BLUR_PIPELINE_H_
#define _BLUR_PIPELINE_H_

#include "../render_pipeline.h"




class ShadowBlurPipeline : public RenderPipeline
{
public:
	enum ETextureType : uint32_t
	{
		eSrc = 0,
		eDstX,
		eDstY,
	};

	enum EBufferType : uint32_t
	{
		eGaussWeights = 0,
	};

	enum EPassType : uint32_t
	{
		eBlurX = 0,
		eBlurY,
	};

	ShadowBlurPipeline();
	~ShadowBlurPipeline();

	virtual void initialize(
		class RenderEngine* engine,
		class RenderPass* pass,
		const std::vector<class Texture*>& textures,
		const std::vector<class Buffer*>& buffers,
		const std::vector<class Mesh*>& meshes) override;
	virtual void cleanup(class RenderEngine* engine) override;

	virtual void render(class RenderEngine* engine, class RenderPass* pass, uint32_t currentImageIndex) override;
	virtual void update(class RenderEngine* engine, uint32_t currentImageIndex) override;
};


#endif
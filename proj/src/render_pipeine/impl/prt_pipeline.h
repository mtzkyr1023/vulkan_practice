#ifndef _PRT_PIPELINE_H_
#define _PRT_PIPELINE_H_


#include "../render_pipeline.h"

class PrtPipeline : public RenderPipeline
{
	enum ETextureType : uint32_t
	{
		eCubemap = 0,
	};

	enum EBufferType : uint32_t
	{
		eShCoeffs = 0,
	};

	enum EPassType : uint32_t
	{
		eSh = 0,
	};

	PrtPipeline();
	~PrtPipeline();

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
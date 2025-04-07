#ifndef _SHADOW_PIPELINE_H_
#define _SHADOW_PIPELINE_H_

#include "../render_pipeline.h"


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

	virtual void initialize(class RenderEngine* engine, class RenderPass* pass, class RenderPass* prePass) override;
	virtual void cleanup(class RenderEngine* engine) override;

	virtual void render(class RenderEngine* engine, class RenderPass* pass, uint32_t currentImageIndex) override;
	virtual void update(class RenderEngine* engine, uint32_t currentImagendex) override;

protected:

};



#endif
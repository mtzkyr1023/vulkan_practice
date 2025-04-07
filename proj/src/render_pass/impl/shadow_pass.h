#ifndef _SHADOW_PASS_H_
#define _SHADOW_PASS_H_

#include "../render_pass.h"


class ShadowPass : public RenderPass
{
public:
	enum ETextureType : uint32_t
	{
		eRaw = 0,
		eBlurX,
		eBlurY,
		eResult,

		eDepth,

		eNum,
	};

private:
	virtual void setupInternal(class RenderEngine* engine) override;
};

#endif
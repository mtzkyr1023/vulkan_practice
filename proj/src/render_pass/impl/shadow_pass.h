#ifndef _SHADOW_PASS_H_
#define _SHADOW_PASS_H_

#include "../render_pass.h"


class ShadowPass : public RenderPass
{
public:
	enum ETextureType : uint32_t
	{
		eRaw = 0,

		eDepth,

		eNum,
	};

private:
	virtual void setupInternal(class RenderEngine* engine, const std::vector<class Texture*>& resources) override;
};

#endif
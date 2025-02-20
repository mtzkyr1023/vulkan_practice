#ifndef _DEFERRED_PASS_H_
#define _DEFERRED_PASS_H_


#include "../render_pass.h"


class DeferredPass : public RenderPass
{
public:
	enum ETextureType : uint32_t
	{
		eResult = 0,
		eAlbedo,
		eNormalDepth,
		eRoughMetalVelocity,
		eTemporary,

		eDepth,
		eStencil,

		eNum,
	};

private:
	virtual void setupInternal(class RenderEngine* engine) override;
};

#endif
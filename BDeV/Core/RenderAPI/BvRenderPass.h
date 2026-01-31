#pragma once


#include "BvRenderCommon.h"


class IBvRenderPass : public BvRCObj
{
	BV_NOCOPYMOVE(IBvRenderPass);

public:
	virtual const RenderPassDesc& GetDesc() const = 0;

protected:
	IBvRenderPass() {}
	~IBvRenderPass() {}
};
BV_OBJECT_DEFINE_ID(IBvRenderPass, "cdaf2dfb-3ab4-458f-846b-f560d415a023");
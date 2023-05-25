#pragma once


#include "BDeV/Utils/BvUtils.h"


class BvContextGl
{
	BV_NOCOPYMOVE(BvContextGl);

protected:
	BvContextGl() {}
	virtual ~BvContextGl() = 0 {}
};


namespace BvRenderGl
{
	BvContextGl* CreateGLContext();
	void DestroyGLContext();
}
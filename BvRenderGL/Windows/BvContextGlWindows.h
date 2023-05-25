#pragma once


#include "BvRenderGL/BvContextGl.h"


class BvContextGlWindows final : public BvContextGl
{
	BV_NOCOPYMOVE(BvContextGlWindows);

public:
	BvContextGlWindows();
	~BvContextGlWindows();

private:
	void Create();
	void Destroy();
};
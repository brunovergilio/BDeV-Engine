#pragma once


#include "BvCommonGl.h"
#include "BvGPUInfoGl.h"


class BvWindow;


class BvContextGl
{
	BV_NOCOPYMOVE(BvContextGl);

public:
	BvContextGl(BvWindow* pWindow);
	~BvContextGl();
	void SwapBuffers(i32 swapInterval);

private:
	void Create();
	void Destroy();

private:
	BvWindow* m_pWindow = nullptr;
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	HDC m_hDC = nullptr;
	HGLRC m_hRC = nullptr;
#endif
	bool m_SupportsVSync = false;
	i32 m_SwapInterval = 0;
};


bool InitializeOpenGL(BvGPUInfoGl& gpuInfo);
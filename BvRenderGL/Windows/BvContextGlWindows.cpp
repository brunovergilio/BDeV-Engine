#include "BvContextGlWindows.h"
#include <Windows.h>


// Use discrete GPU by default.
extern "C"
{
	// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

	// http://developer.amd.com/community/blog/2015/10/02/amd-enduro-system-for-developers/
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}


BvContextGlWindows::BvContextGlWindows()
{
	Create();
}


BvContextGlWindows::~BvContextGlWindows()
{
	Destroy();
}


void BvContextGlWindows::Create()
{

}


void BvContextGlWindows::Destroy()
{

}


namespace BvRenderGl
{
	static BvContextGlWindows* s_pContextGL = nullptr;

	BvContextGl* CreateGLContext()
	{
		if (!s_pContextGL)
		{
			s_pContextGL = new BvContextGlWindows();
		}

		return s_pContextGL;
	}

	void DestroyGLContext()
	{
		delete s_pContextGL;
		s_pContextGL = nullptr;
	}
}
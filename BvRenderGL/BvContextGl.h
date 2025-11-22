#pragma once


#include "BvCommonGl.h"
#include "BDeV/Core/System/Threading/BvSync.h"


class BvWindow;


class BvContextGl
{
	BV_NOCOPYMOVE(BvContextGl);

public:
	BvContextGl(BvWindow* pWindow);
	~BvContextGl();
	void SwapBuffers(i32 swapInterval);

	static BvContextGl*& GetCurrent();
	void MakeCurrent();
	void ReleaseCurrent();
	bool SupportsSRGB() const;

	BV_INLINE bool IsValid() const { return m_hRC != nullptr; }

private:
	void Flush();
	void Create();
	void Destroy();

private:
	BvWindow* m_pWindow = nullptr;
#if BV_PLATFORM_WIN32
	HWND m_hWnd = nullptr; // Only used by the master context
	HDC m_hDC = nullptr;
	HGLRC m_hRC = nullptr;
#endif
	bool m_SupportsVSync = false;
	i32 m_SwapInterval = 0;
};


template<typename LockType>
class BvScopedContextGl
{
	BV_NOCOPYMOVE(BvScopedContextGl);
public:
	BvScopedContextGl(BvContextGl* pContext, LockType& lock)
		: m_pContext(pContext), m_Lock(lock), m_NeedsHandling(BvContextGl::GetCurrent() == nullptr)
	{
		if (m_NeedsHandling)
		{
			m_Lock.Lock();
			m_pContext->MakeCurrent();
		}
	}

	~BvScopedContextGl()
	{
		if (m_NeedsHandling)
		{
			m_pContext->ReleaseCurrent();
			m_Lock.Unlock();
		}
	}

private:
	BvContextGl* m_pContext;
	LockType& m_Lock;
	bool m_NeedsHandling;
};


bool InitializeOpenGL(BvDeviceInfoGl& gpuInfo);
void ShutdownOpenGL();
#include "BvRenderGL/BvContextGl.h"
#include "GL/glew.h"
#include "GL/wglew.h"
#include "BDeV/Core/System/Window/BvWindow.h"
#include "BDeV/Core/Container/BvFixedVector.h"


// Use discrete GPU by default.
extern "C"
{
	// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

	// http://developer.amd.com/community/blog/2015/10/02/amd-enduro-system-for-developers/
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}


constexpr LPCSTR g_TempContextClassName = "OpenGL Temp Context";


HWND CreateWindow()
{
	// =======================================
	// Create a temporary window
	auto hInstance = GetModuleHandle(nullptr);
	//#ifdef BV_STATIC_LIB
	//#else
	//	auto hInstance = GetModuleHandle("BvRenderGl.dll");
	//#endif

	HWND hWnd = CreateWindowA(g_TempContextClassName, g_TempContextClassName, 0, 0, 0, 32, 32, 0, 0, hInstance, 0);
	if (!hWnd)
	{
		BV_SYS_ERROR();
	}
	
	return hWnd;
}


LRESULT TempWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, msg, wParam, lParam);
}


BvContextGl* g_pMasterContext = nullptr;


BvContextGl::BvContextGl(BvWindow* pWindow)
	: m_pWindow(pWindow)
{
	Create();
}


BvContextGl::~BvContextGl()
{
	Destroy();
}


void BvContextGl::SwapBuffers(i32 swapInterval)
{
	if (m_SupportsVSync && m_SwapInterval != swapInterval)
	{
		m_SwapInterval = swapInterval;
		wglSwapIntervalEXT(m_SwapInterval);
	}

	if (!::SwapBuffers(m_hDC))
	{
		BV_SYS_ERROR();
	}
}


BvContextGl*& BvContextGl::GetCurrent()
{
	static thread_local BvContextGl* pContext = nullptr;
	return pContext;
}


void BvContextGl::MakeCurrent()
{
	auto& pCurrent = GetCurrent();
	if (pCurrent != this)
	{
		if (pCurrent)
		{
			Flush();
		}

		if (!wglMakeCurrent(m_hDC, m_hRC))
		{
			BV_ERROR("OpenGL", "Couldn't set new context");
		}
		pCurrent = this;
	}
}


void BvContextGl::ReleaseCurrent()
{
	auto& pCurrent = GetCurrent();
	if (pCurrent == this)
	{
		Flush();

		if (!wglMakeCurrent(nullptr, nullptr))
		{
			BV_ERROR("OpenGL", "Couldn't release context");
		}
		pCurrent = nullptr;
	}
}


bool BvContextGl::SupportsSRGB() const
{
	return WGLEW_EXT_framebuffer_sRGB;
}


void BvContextGl::Flush()
{
	glFlush();
}


void BvContextGl::Create()
{
	bool setPixelFormat = true;
	HGLRC hRC = nullptr;
	if (m_pWindow)
	{
		m_hDC = ::GetDC(m_pWindow->GetHandle());
		hRC = g_pMasterContext->m_hRC;
	}
	else if (g_pMasterContext)
	{
		m_hDC = g_pMasterContext->m_hDC;
		hRC = g_pMasterContext->m_hRC;

		setPixelFormat = false;
	}
	else
	{
		// Only the main context needs a dummy window, others can reuse it
		m_hWnd = CreateWindow();
		m_hDC = ::GetDC(m_hWnd);
	}

	if (WGLEW_EXT_swap_control)
	{
		m_SupportsVSync = true;
	}

	if (setPixelFormat && !::GetPixelFormat(m_hDC))
	{
		constexpr auto kMaxPixelFormatAttribs = 23; // Increase as needed
		BvFixedVector<i32, kMaxPixelFormatAttribs> pixelFormatAttribs;
		pixelFormatAttribs.EmplaceBack(WGL_DRAW_TO_WINDOW_ARB); pixelFormatAttribs.EmplaceBack(GL_TRUE);
		pixelFormatAttribs.EmplaceBack(WGL_SUPPORT_OPENGL_ARB); pixelFormatAttribs.EmplaceBack(GL_TRUE);
		pixelFormatAttribs.EmplaceBack(WGL_DOUBLE_BUFFER_ARB); pixelFormatAttribs.EmplaceBack(GL_TRUE);
		pixelFormatAttribs.EmplaceBack(WGL_SAMPLE_BUFFERS_ARB); pixelFormatAttribs.EmplaceBack(GL_TRUE);
		pixelFormatAttribs.EmplaceBack(WGL_ACCELERATION_ARB); pixelFormatAttribs.EmplaceBack(WGL_FULL_ACCELERATION_ARB);
		pixelFormatAttribs.EmplaceBack(WGL_PIXEL_TYPE_ARB); pixelFormatAttribs.EmplaceBack(WGL_TYPE_RGBA_ARB);
		pixelFormatAttribs.EmplaceBack(WGL_COLOR_BITS_ARB); pixelFormatAttribs.EmplaceBack(24);
		pixelFormatAttribs.EmplaceBack(WGL_ALPHA_BITS_ARB); pixelFormatAttribs.EmplaceBack(8);
		//pixelFormatAttribs.EmplaceBack(WGL_DEPTH_BITS_ARB); pixelFormatAttribs.EmplaceBack(24);
		//pixelFormatAttribs.EmplaceBack(WGL_STENCIL_BITS_ARB); pixelFormatAttribs.EmplaceBack(8);

		if (WGLEW_EXT_framebuffer_sRGB)
		{
			pixelFormatAttribs.EmplaceBack(WGL_FRAMEBUFFER_SRGB_CAPABLE_EXT); pixelFormatAttribs.EmplaceBack(GL_TRUE);
		}

		pixelFormatAttribs.EmplaceBack(0);

		int pixelFormat{};
		UINT numFormats{};
		wglChoosePixelFormatARB(m_hDC, pixelFormatAttribs.Data(), 0, 1, &pixelFormat, &numFormats);
		if (!numFormats)
		{
			BV_ERROR("WGL", "Failed to choose the pixel format.\n");
		}

		PIXELFORMATDESCRIPTOR pfd;
		if (!::DescribePixelFormat(m_hDC, pixelFormat, sizeof(pfd), &pfd))
		{
			BV_ERROR("WGL", "Failed to describe the pixel format.\n");
		}
		if (!::SetPixelFormat(m_hDC, pixelFormat, &pfd))
		{
			BV_ERROR("WGL", "Failed to set the pixel format.\n");
		}
	}

	// Setup attributes for a new OpenGL rendering context
	constexpr auto kMaxAttribs = 11; // Increase as needed
	BvFixedVector<i32, kMaxAttribs> contextAttribs;
	contextAttribs.EmplaceBack(WGL_CONTEXT_MAJOR_VERSION_ARB);	contextAttribs.EmplaceBack(0);
	contextAttribs.EmplaceBack(WGL_CONTEXT_MINOR_VERSION_ARB);	contextAttribs.EmplaceBack(0);
	contextAttribs.EmplaceBack(WGL_CONTEXT_FLAGS_ARB);	contextAttribs.EmplaceBack(WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB);
	contextAttribs.EmplaceBack(WGL_CONTEXT_PROFILE_MASK_ARB);	contextAttribs.EmplaceBack(WGL_CONTEXT_CORE_PROFILE_BIT_ARB);

#if (BV_DEBUG)
	contextAttribs[5] |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

	// This allows us to call wglMakeCurrent() without causing a flush of the commands
	if (GLEW_KHR_context_flush_control && WGLEW_ARB_context_flush_control)
	{
		contextAttribs.EmplaceBack(WGL_CONTEXT_RELEASE_BEHAVIOR_ARB); contextAttribs.EmplaceBack(WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB);
	}

	contextAttribs.EmplaceBack(0);

	std::pair<int, int> availableVersions[] = { { 4, 6 }, { 4, 5 }, { 4, 3 }, { 4, 0 } };
	constexpr auto kVersionCount = ArraySize(availableVersions);
	for (auto i = 0; i < kVersionCount && m_hRC == nullptr; i++)
	{
		const auto& version = availableVersions[i];
		contextAttribs[1] = version.first;
		contextAttribs[3] = version.second;

		m_hRC = wglCreateContextAttribsARB(m_hDC, hRC, contextAttribs.Data());
		if (!m_hRC)
		{
			switch (glGetError())
			{
			case GL_INVALID_OPERATION:			BV_ERROR("WGL", "Couldn't create an extended OpenGL context!\n"); break;
			case ERROR_INVALID_VERSION_ARB:		//BV_ERROR("WGL", "ERROR_INVALID_VERSION_ARB\n"); break;
			case ERROR_INVALID_PROFILE_ARB:		//BV_ERROR("WGL", "ERROR_INVALID_PROFILE_ARB\n"); break;
			case ERROR_INVALID_PIXEL_TYPE_ARB:	//BV_ERROR("WGL", "ERROR_INVALID_PIXEL_TYPE_ARB\n"); break;
			default:
				break;
			}
		}
	}

	if (!m_hRC)
	{
		BV_ERROR("WGL", "OpenGL 4.0+ not supported");
		return;
	}

	if (m_hWnd)
	{
		g_pMasterContext = this;
	}

	if (!m_pWindow)
	{
		MakeCurrent();
	}
}


void BvContextGl::Destroy()
{
	if (m_hRC)
	{
		ReleaseCurrent();
		
		wglDeleteContext(m_hRC);
		m_hRC = nullptr;

		if (m_pWindow)
		{
			::ReleaseDC(m_pWindow->GetHandle(), m_hDC);
		}
		else if (m_hWnd)
		{
			::ReleaseDC(m_hWnd, m_hDC);
			::DestroyWindow(m_hWnd);
		}

		m_hWnd = nullptr;
		m_hDC = nullptr;
	}
}


bool InitializeOpenGL(BvDeviceInfoGl& gpuInfo)
{
	// =======================================
	// Create a temporary window
	auto hInstance = GetModuleHandle(nullptr);
	//#ifdef BV_STATIC_LIB
	//#else
	//	auto hInstance = GetModuleHandle("BvRenderGl.dll");
	//#endif

	WNDCLASSA wndClass;
	memset(&wndClass, 0, sizeof(WNDCLASSA));
	wndClass.style = CS_OWNDC;
	wndClass.hInstance = hInstance;
	wndClass.lpfnWndProc = TempWndProc;
	wndClass.lpszClassName = g_TempContextClassName;
	RegisterClassA(&wndClass);

	HWND hTempWnd = CreateWindow();
	if (!hTempWnd)
	{
		return false;
	}

	auto hTempDC = GetDC(hTempWnd);

	// =======================================
	// Choose and set the pixel format
	PIXELFORMATDESCRIPTOR pfd{ sizeof(PIXELFORMATDESCRIPTOR) };
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cAlphaBits = 8;
	//pfd.cDepthBits = 24;
	//pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	auto pixelFormat = ::ChoosePixelFormat(hTempDC, &pfd);
	if (!pixelFormat)
	{
		BV_SYS_ERROR();
		return false;
	}
	auto pixelFormatResult = ::SetPixelFormat(hTempDC, pixelFormat, &pfd);
	if (!pixelFormatResult)
	{
		BV_SYS_ERROR();
		return false;
	}

	// =======================================
	// Create the temporary GL Context and query for the ARB context
	HGLRC hTempContext = wglCreateContext(hTempDC);
	if (!hTempContext)
	{
		BV_SYS_ERROR();
		return false;
	}

	if (!wglMakeCurrent(hTempDC, hTempContext))
	{
		BV_SYS_ERROR();
		return false;
	}

	// Load up GLEW and make sure it supports an extended context
	auto glewResult = glewInit();
	if (glewResult != GLEW_OK)
	{
		BV_ERROR("WGL", "Failed to initialize GLEW");
		return false;
	}

	if (!WGLEW_ARB_create_context)
	{
		BV_ERROR("WGL", "OpenGL 4.0+ not supported");
		return false;
	}

	const char* pVersionName = (const char*)glGetString(GL_VERSION);
	const char* pRendererName = (const char*)glGetString(GL_RENDERER);
	const char* pVendorName = (const char*)glGetString(GL_VENDOR);
	const char* pShaderVersionName = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	const char* pDot = strstr(pVersionName, ".");
	u32 majorVersion = 0;
	u32 minorVersion = 0;
	if (pDot)
	{
		auto index = (u32)(pDot - pVersionName);
		majorVersion = (u32)(pVersionName[index - 1] - '0');
		minorVersion = (u32)(pVersionName[index + 1] - '0');
	}

	gpuInfo.majorVersion = majorVersion;
	gpuInfo.minorVersion = minorVersion;
	strncpy(gpuInfo.driverName, pVersionName, BvDeviceInfoGl::kMaxDriverNameSize - 1);
	strncpy(gpuInfo.deviceName, pRendererName, BvDeviceInfoGl::kMaxDeviceNameSize - 1);
	strncpy(gpuInfo.vendorName, pVendorName, BvDeviceInfoGl::kMaxDeviceNameSize - 1);
	strncpy(gpuInfo.shaderVersionName, pShaderVersionName, BvDeviceInfoGl::kMaxDeviceNameSize - 1);

	// Check for extended features
	gpuInfo.m_ExtendedFeatures.textureFilterAnisotropic = GLEW_EXT_texture_filter_anisotropic;
	gpuInfo.m_ExtendedFeatures.polygonOffsetClamp = GLEW_EXT_polygon_offset_clamp;
	gpuInfo.m_ExtendedFeatures.nvConservativeRaster = GLEW_NV_conservative_raster;
	gpuInfo.m_ExtendedFeatures.intelConservativeRaster = GLEW_INTEL_conservative_rasterization;
	gpuInfo.m_ExtendedFeatures.depthBoundsTest = GLEW_EXT_depth_bounds_test;
	gpuInfo.m_ExtendedFeatures.directStateAccess = GLEW_ARB_direct_state_access;

	if (!wglMakeCurrent(nullptr, nullptr))
	{
		BV_SYS_ERROR();
		return false;
	}

	if (!wglDeleteContext(hTempContext))
	{
		BV_SYS_ERROR();
		return false;
	}

	if (!::ReleaseDC(hTempWnd, hTempDC))
	{
		BV_SYS_ERROR();
		return false;
	}

	if (!::DestroyWindow(hTempWnd))
	{
		BV_SYS_ERROR();
		return false;
	}

	return true;
}


void ShutdownOpenGL()
{
	wglMakeCurrent(nullptr, nullptr);

	UnregisterClassA(g_TempContextClassName, GetModuleHandle(nullptr));
}
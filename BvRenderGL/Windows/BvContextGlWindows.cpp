#include "BvRenderGL/BvContextGl.h"
#include <GL/glew.h>
#include <GL/wglew.h>
#include "BDeV/System/Window/BvWindow.h"


// Use discrete GPU by default.
extern "C"
{
	// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

	// http://developer.amd.com/community/blog/2015/10/02/amd-enduro-system-for-developers/
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}


LPCSTR g_TempContextClassName = "OpenGLContextName";


LRESULT TempWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, msg, wParam, lParam);
}


BvContextGl::BvContextGl(BvWindow* pWindow)
	: m_hDC(pWindow->GetDC())
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
		BV_OS_ERROR();
	}
}


void BvContextGl::Create()
{
	int pixelFormatAttribs[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB,     GL_TRUE,
		WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,         24,
		WGL_ALPHA_BITS_ARB,         8,
		WGL_DEPTH_BITS_ARB,         24,
		WGL_STENCIL_BITS_ARB,       8,
		0,							0,
		0
	};

	if (wglewIsSupported("WGL_EXT_framebuffer_sRGB"))
	{
		pixelFormatAttribs[20] = WGL_FRAMEBUFFER_SRGB_CAPABLE_EXT;
		pixelFormatAttribs[21] = GL_TRUE;
	}

	if (wglewIsSupported("WGL_EXT_swap_control"))
	{
		m_SupportsVSync = true;
	}

	int pixelFormat{};
	UINT numFormats{};
	wglChoosePixelFormatARB(m_hDC, pixelFormatAttribs, 0, 1, &pixelFormat, &numFormats);
	if (!numFormats)
	{
		BV_ERROR("Failed to choose the pixel format.\n");
	}

	PIXELFORMATDESCRIPTOR pfd;
	if (!DescribePixelFormat(m_hDC, pixelFormat, sizeof(pfd), &pfd))
	{
		BV_ERROR("Failed to describe the pixel format.\n");
	}
	if (!SetPixelFormat(m_hDC, pixelFormat, &pfd))
	{
		BV_ERROR("Failed to set the pixel format.\n");
	}

	auto majorVersion = 0;
	auto minorVersion = 0;

	std::pair<int, int> availableVersions[] = { { 4, 6 }, { 4, 5 } };
	for (auto i = 0; i < 2 && m_hRC == nullptr; i++)
	{
		const auto& version = availableVersions[i];
		majorVersion = version.first;
		minorVersion = version.second;
		// Setup attributes for a new OpenGL rendering context
		int attribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, majorVersion,
			WGL_CONTEXT_MINOR_VERSION_ARB, minorVersion,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0,
		};

#if (BV_DEBUG)
		attribs[5] |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

		m_hRC = wglCreateContextAttribsARB(m_hDC, 0, attribs);
		if (!m_hRC)
		{
			switch (glGetError())
			{
			case GL_INVALID_OPERATION:			BV_ERROR("Couldn't create an extended OpenGL context!\n"); break;
			case ERROR_INVALID_VERSION_ARB:		//BV_ERROR("ERROR_INVALID_VERSION_ARB\n"); break;
			case ERROR_INVALID_PROFILE_ARB:		//BV_ERROR("ERROR_INVALID_PROFILE_ARB\n"); break;
			case ERROR_INVALID_PIXEL_TYPE_ARB:	//BV_ERROR("ERROR_INVALID_PIXEL_TYPE_ARB\n"); break;
			default:
				break;
			}
		}
	}

	if (!m_hRC)
	{
		BV_ERROR("OpenGL 4.0+ not supported");
		return;
	}

	if (!wglMakeCurrent(m_hDC, m_hRC))
	{
		BV_OS_ERROR();
	}
}


void BvContextGl::Destroy()
{
	if (m_hRC)
	{
		wglMakeCurrent(m_hDC, nullptr);
		wglDeleteContext(m_hRC);
		m_hRC = nullptr;
		m_hDC = nullptr;
	}
}


bool InitializeOpenGL(BvGPUInfoGl& gpuInfo)
{
	// =======================================
	// Create a temporary window
#ifdef BV_STATIC_LIB
	auto hInstance = GetModuleHandle(nullptr);
#else
	auto hInstance = GetModuleHandle("BvRenderGl.dll");
#endif

	WNDCLASS dummyClass;
	memset(&dummyClass, 0, sizeof(WNDCLASS));
	dummyClass.style = CS_OWNDC;
	dummyClass.hInstance = hInstance;
	dummyClass.lpfnWndProc = TempWndProc;
	dummyClass.lpszClassName = g_TempContextClassName;
	RegisterClassA(&dummyClass);

	HWND hTempWnd = CreateWindowA(g_TempContextClassName, g_TempContextClassName, 0, 0, 0, 32, 32, 0, 0, hInstance, 0);
	if (!hTempWnd)
	{
		BV_OS_ERROR();
		return false;
	}

	auto hTempDC = GetDC(hTempWnd);

	// =======================================
	// Choose and set the pixel format
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	auto pixelFormat = ChoosePixelFormat(hTempDC, &pfd);
	if (!pixelFormat)
	{
		BV_OS_ERROR();
		return false;
	}
	auto pixelFormatResult = SetPixelFormat(hTempDC, pixelFormat, &pfd);
	if (!pixelFormatResult)
	{
		BV_OS_ERROR();
		return false;
	}

	// =======================================
	// Create the temporary GL Context and query for the ARB context
	HGLRC hTempContext = wglCreateContext(hTempDC);
	if (!hTempContext)
	{
		BV_OS_ERROR();
		return false;
	}

	if (!wglMakeCurrent(hTempDC, hTempContext))
	{
		BV_OS_ERROR();
		return false;
	}

	// Load up GLEW and make sure it supports an extended context
	auto glewResult = glewInit();
	if (glewResult != GLEW_OK)
	{
		BV_ERROR("Failed to initialize GLEW");
		return false;
	}

	if (!WGLEW_ARB_create_context)
	{
		BV_ERROR("OpenGL 4.0+ not supported");
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
	strncpy(gpuInfo.driverName, pVersionName, BvGPUInfoGl::kMaxDriverNameSize - 1);
	strncpy(gpuInfo.deviceName, pRendererName, BvGPUInfoGl::kMaxDeviceNameSize - 1);
	strncpy(gpuInfo.vendorName, pVendorName, BvGPUInfoGl::kMaxDeviceNameSize - 1);
	strncpy(gpuInfo.shaderVersionName, pShaderVersionName, BvGPUInfoGl::kMaxDeviceNameSize - 1);

	// Check for extended features
	gpuInfo.m_ExtendedFeatures.textureFilterAnisotropic = GLEW_EXT_texture_filter_anisotropic;
	gpuInfo.m_ExtendedFeatures.polygonOffsetClamp = GLEW_EXT_polygon_offset_clamp;
	gpuInfo.m_ExtendedFeatures.nvConservativeRaster = GLEW_NV_conservative_raster;
	gpuInfo.m_ExtendedFeatures.intelConservativeRaster = GLEW_INTEL_conservative_rasterization;
	gpuInfo.m_ExtendedFeatures.depthBoundsTest = GLEW_EXT_depth_bounds_test;
	gpuInfo.m_ExtendedFeatures.directStateAccess = GLEW_ARB_direct_state_access;

	if (!wglMakeCurrent(nullptr, nullptr))
	{
		BV_OS_ERROR();
		return false;
	}

	if (!wglDeleteContext(hTempContext))
	{
		BV_OS_ERROR();
		return false;
	}

	if (!ReleaseDC(hTempWnd, hTempDC))
	{
		BV_OS_ERROR();
		return false;
	}

	if (!DestroyWindow(hTempWnd))
	{
		BV_OS_ERROR();
		return false;
	}

	if (!UnregisterClassA(g_TempContextClassName, hInstance))
	{
		BV_OS_ERROR();
		return false;
	}

	return true;
}
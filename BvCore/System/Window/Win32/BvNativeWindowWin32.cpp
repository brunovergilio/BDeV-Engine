#include "BvCore/System/Window/BvNativeWindow.h"
#include "BvCore/BvDebug.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


constexpr const DWORD kWindowedStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
constexpr const DWORD kWindowedExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

constexpr const DWORD kFullscreenStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
constexpr const DWORD kFullscreenExStyle = WS_EX_APPWINDOW;


BvNativeWindow::BvNativeWindow(const WindowDesc& windowDesc)
	: m_WindowDesc(windowDesc), m_WindowedWidth(windowDesc.m_Width), m_WindowedHeight(windowDesc.m_Height)
{
	Create();
}


BvNativeWindow::BvNativeWindow(BvNativeWindow&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvNativeWindow& BvNativeWindow::operator=(BvNativeWindow&& rhs) noexcept
{
	if (this != &rhs)
	{
		m_hWnd = rhs.m_hWnd; rhs.m_hWnd = nullptr;
		m_SizeChangedCallbacks = std::move(rhs.m_SizeChangedCallbacks);

		m_WindowDesc = std::move(rhs.m_WindowDesc);

		m_FullscreenWidth = rhs.m_FullscreenWidth;
		m_FullscreenHeight = rhs.m_FullscreenHeight;

		m_WindowedWidth = rhs.m_WindowedWidth;
		m_WindowedHeight = rhs.m_WindowedHeight;
		m_WindowedPosX = rhs.m_WindowedPosX;
		m_WindowedPosY = rhs.m_WindowedPosY;

		m_IsPaused = rhs.m_IsPaused;
		m_IsResizing = rhs.m_IsResizing;
		m_IsMinimized = rhs.m_IsMinimized;
		m_IsMaximized = rhs.m_IsMaximized;
	}

	return *this;
}


BvNativeWindow::~BvNativeWindow()
{
	Destroy();
}


void BvNativeWindow::Create()
{
	{
		//DEVMODE devMode = {};
		//devMode.dmSize = sizeof(DEVMODE);
		//if (!EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode))
		//{
		//	BV_WIN32_ERROR();
		//}
		//SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
		m_FullscreenWidth =  GetSystemMetrics(SM_CXSCREEN);//devMode.dmPelsWidth;
		m_FullscreenHeight = GetSystemMetrics(SM_CYSCREEN);//devMode.dmPelsHeight;
	}

	HINSTANCE hInstance = GetModuleHandle(nullptr);

	{
		// Register the window class.
		WNDCLASSEX wndClass = {};
		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = StaticWndProc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = hInstance;
		wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndClass.lpszMenuName = nullptr;
		wndClass.lpszClassName = m_WindowDesc.m_Name.CStr();
		wndClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

		if (!RegisterClassEx(&wndClass))
		{
			BV_WIN32_ERROR();
		}
	}

	DWORD style;
	DWORD exStyle;
	WindowDesc modeDesc;
	GetWindowDescForMode(m_WindowDesc.m_WindowMode, modeDesc, style, exStyle);

	m_hWnd = CreateWindowEx(exStyle, m_WindowDesc.m_Name.CStr(), m_WindowDesc.m_Name.CStr(), style,
		modeDesc.m_PosX, modeDesc.m_PosY, modeDesc.m_Width, modeDesc.m_Height, nullptr, nullptr, hInstance, this);
	if (!m_hWnd)
	{
		BV_WIN32_ERROR();
	}

	SetTitle(m_WindowDesc.m_Name.CStr());
}


void BvNativeWindow::Destroy()
{
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
		UnregisterClass(m_WindowDesc.m_Name.CStr(), GetModuleHandle(nullptr));
	}
}


void BvNativeWindow::ChangeMode(const WindowMode windowMode)
{
	if (m_WindowDesc.m_WindowMode == windowMode)
	{
		return;
	}

	// If the current mode is kWindowed, backup its current dimensions
	if (m_WindowDesc.m_WindowMode == WindowMode::kWindowed)
	{
		m_WindowedWidth = m_WindowDesc.m_Width;
		m_WindowedHeight = m_WindowDesc.m_Height;
		m_WindowedPosX = m_WindowDesc.m_PosX;
		m_WindowedPosY = m_WindowDesc.m_PosY;
	}

	DWORD style;
	DWORD exStyle;
	WindowDesc modeDesc;
	GetWindowDescForMode(windowMode, modeDesc, style, exStyle);

	SetWindowLongPtr(m_hWnd, GWL_STYLE, style);
	SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, exStyle);
	SetWindowPos(m_hWnd, nullptr, modeDesc.m_PosX, modeDesc.m_PosY,
		modeDesc.m_Width, modeDesc.m_Height, SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOZORDER);
	
	m_WindowDesc.m_WindowMode = windowMode;
}


void BvNativeWindow::Resize(const u32 width, const u32 height)
{
	if (m_WindowDesc.m_WindowMode != WindowMode::kWindowed
		|| (width == m_WindowDesc.m_Width && height == m_WindowDesc.m_Height))
	{
		return;
	}

	RECT windowRect{};
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = static_cast<LONG>(width);
	windowRect.bottom = static_cast<LONG>(height);
	AdjustWindowRectEx(&windowRect, kWindowedStyle, FALSE, kWindowedExStyle);

	SetWindowPos(m_hWnd, nullptr, 0, 0,
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE);
}


void BvNativeWindow::Move(const i32 posX, const i32 posY)
{
	if (m_WindowDesc.m_WindowMode != WindowMode::kWindowed
		|| (posX == m_WindowDesc.m_PosX && posY == m_WindowDesc.m_PosY))
	{
		return;
	}

	SetWindowPos(m_hWnd, nullptr, posX, posY,
		0, 0, SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
}


void BvNativeWindow::Show()
{
	BvAssertMsg(m_hWnd != nullptr, "Window Handle can't be null");
	ShowWindow(m_hWnd, SW_SHOW);
	m_IsPaused = false;
}


void BvNativeWindow::Hide()
{
	BvAssertMsg(m_hWnd != nullptr, "Window Handle can't be null");
	m_IsPaused = false;
	ShowWindow(m_hWnd, SW_HIDE);
}


void BvNativeWindow::SetTitle(const char* const pTitle)
{
	BvAssertMsg(m_hWnd != nullptr, "Window Handle can't be null");
	SetWindowText(m_hWnd, pTitle);
}


LRESULT BvNativeWindow::StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BvNativeWindow *pWindow = nullptr;

	if (uMsg == WM_NCCREATE)
	{
		pWindow = reinterpret_cast<BvNativeWindow *>((reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		pWindow->m_hWnd = hwnd;

		return TRUE;
	}
	else
	{
		pWindow = reinterpret_cast<BvNativeWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		return pWindow->WndProc(hwnd, uMsg, wParam, lParam);
	}
}


LRESULT BvNativeWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_IsPaused = true;
		}
		else
		{
			m_IsPaused = false;
		}
		return 0;

	case WM_SIZE:
	{
		m_WindowDesc.m_Width = LOWORD(lParam);
		m_WindowDesc.m_Height = HIWORD(lParam);

		if (wParam == SIZE_MINIMIZED)
		{
			m_IsPaused = true;
			m_IsMinimized = true;
			m_IsMaximized = false;
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			m_IsPaused = false;
			m_IsResizing = false;
			m_IsMinimized = false;
			m_IsMaximized = true;

			NotifySizeChanged();
		}
		else if (wParam == SIZE_RESTORED)
		{
			if (m_IsMinimized)
			{
				m_IsPaused = false;
				m_IsResizing = false;
				m_IsMinimized = false;
				NotifySizeChanged();
			}
			else if (m_IsMaximized)
			{
				m_IsPaused = false;
				m_IsResizing = false;
				m_IsMaximized = false;
				NotifySizeChanged();
			}
			//else if (!m_IsResizing)
			//{
			//	NotifyWindowResize();
			//}
		}
	}
	return 0;

	case WM_ENTERSIZEMOVE:
		m_IsPaused = true;
		m_IsResizing = true;
		return 0;

	case WM_EXITSIZEMOVE:
		m_IsResizing = false;
		m_IsPaused = false;
		NotifySizeChanged();
		return 0;

	case WM_MOVE:
	{
		if (m_WindowDesc.m_WindowMode == WindowMode::kWindowed)
		{
			m_WindowDesc.m_PosX = (i32)(i16)LOWORD(lParam);
			m_WindowDesc.m_PosY = (i32)(i16)HIWORD(lParam);
		}
		
		return 0;
	}

	case WM_MOVING:
	{
		if (m_WindowDesc.m_WindowMode == WindowMode::kWindowed)
		{
			auto pRect = (LPRECT)lParam;
			m_WindowDesc.m_PosX = pRect->left;
			m_WindowDesc.m_PosY = pRect->top;
		}

		return TRUE;
	}

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_CLOSE:
		Hide();
		m_IsPaused = false;
		return 0;

	case WM_DESTROY:
		// Don't want to do a PostQuitMessage(0) here, since we may have multiple windows
		// Therefore, there's no need to end the message loop
		return 0;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

void BvNativeWindow::GetWindowDescForMode(const WindowMode windowMode, WindowDesc& windowDesc, DWORD& style, DWORD& exStyle)
{
	switch (windowMode)
	{
	case WindowMode::kWindowed:
	{
		// If we're currently on Windowed mode, we can use the information from WindowDesc
		if (m_WindowDesc.m_WindowMode == WindowMode::kWindowed)
		{
			windowDesc.m_Width = m_WindowDesc.m_Width;
			windowDesc.m_Height = m_WindowDesc.m_Height;
			windowDesc.m_PosX = m_WindowDesc.m_PosX;
			windowDesc.m_PosY = m_WindowDesc.m_PosY;
		}
		// Otherwise, Get it from the windowed size
		else
		{
			windowDesc.m_Width = m_WindowedWidth;
			windowDesc.m_Height = m_WindowedHeight;
			windowDesc.m_PosX = m_WindowedPosX;
			windowDesc.m_PosY = m_WindowedPosY;
		}

		style = kWindowedStyle;
		exStyle = kWindowedExStyle;

		RECT windowRect{};
		windowRect.left = 0L;
		windowRect.top = 0L;
		windowRect.right = static_cast<LONG>(windowDesc.m_Width);
		windowRect.bottom = static_cast<LONG>(windowDesc.m_Height);
		AdjustWindowRectEx(&windowRect, style, FALSE, exStyle);

		windowDesc.m_Width = windowRect.right - windowRect.left;
		windowDesc.m_Height = windowRect.bottom - windowRect.top;

		break;
	}
	case WindowMode::kBordeless:
	{
		windowDesc.m_Width = m_FullscreenWidth;
		windowDesc.m_Height = m_FullscreenHeight;
		windowDesc.m_PosX = 0;
		windowDesc.m_PosY = 0;

		style = kFullscreenStyle;
		exStyle = kFullscreenExStyle;

		break;
	}
	case WindowMode::kFullscreen:
	{
		BvAssertMsg(0, "Not implemented yet");
		break;
	}
	}
}


void BvNativeWindow::NotifySizeChanged()
{
	for (auto && sizeChangedCallback : m_SizeChangedCallbacks)
	{
		sizeChangedCallback.second(m_WindowDesc.m_Width, m_WindowDesc.m_Height);
	}
}


#endif
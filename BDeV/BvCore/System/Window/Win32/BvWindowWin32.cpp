#include "BvCore/System/Window/BvWindow.h"
#include "BvCore/BvDebug.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32)


constexpr char pAppName[] = "BDeV Engine";


BvWindow::BvWindow()
{
}


BvWindow::~BvWindow()
{
	Destroy();
}


void BvWindow::Create(const u32 width, const u32 height)
{
	m_Width = width;
	m_Height = height;

	HINSTANCE hInstance = GetModuleHandle(nullptr);

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
	wndClass.lpszClassName = pAppName;
	wndClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

	ATOM regClassResult = RegisterClassEx(&wndClass);
	BvAssertMsg(regClassResult, "Couldn't register class");

	RECT windowRect;
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = static_cast<LONG>(width);
	windowRect.bottom = static_cast<LONG>(height);

	DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	DWORD exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	AdjustWindowRectEx(&windowRect, style, FALSE, exStyle);

	//RECT desktopWindowRect{};
	//GetClientRect(GetDesktopWindow(), &desktopWindowRect);

	m_hWnd = CreateWindow(pAppName, pAppName, style, 0, 0,
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, hInstance, this);

	BvAssertMsg(m_hWnd != nullptr, "Couldn't create window");

	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);
}


void BvWindow::Destroy()
{
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
		UnregisterClass(pAppName, GetModuleHandle(nullptr));
	}
}


void BvWindow::Show()
{
	BvAssertMsg(m_hWnd != nullptr, "Window Handle can't be null");
	ShowWindow(m_hWnd, SW_SHOW);
}


void BvWindow::Hide()
{
	BvAssertMsg(m_hWnd != nullptr, "Window Handle can't be null");
	ShowWindow(m_hWnd, SW_HIDE);
}


LRESULT BvWindow::StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BvWindow *pWindow = nullptr;

	if (uMsg == WM_NCCREATE)
	{
		pWindow = reinterpret_cast<BvWindow *>((reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
	}
	else
	{
		pWindow = reinterpret_cast<BvWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (pWindow)
		{
			pWindow->WndProc(hwnd, uMsg, wParam, lParam);
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


LRESULT BvWindow::WndProc(HWND /*hwnd*/, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	//case WM_ACTIVATE:
	//	if (LOWORD(wParam) == WA_INACTIVE)
	//	{
	//		m_IsPaused = true;
	//	}
	//	else
	//	{
	//		m_IsPaused = false;
	//	}
	//	return 0;

	case WM_SIZE:
	{
		m_Width = LOWORD(lParam);
		m_Height = HIWORD(lParam);

		if (wParam == SIZE_MINIMIZED)
		{
			m_IsMinimized = true;
			m_IsMaximized = false;
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			m_IsResizing = false;
			m_NeedsUpdate = true;
			m_IsPaused = false;
			m_IsMinimized = false;
			m_IsMaximized = true;
		}
		else if (wParam == SIZE_RESTORED)
		{
			if (m_IsMinimized)
			{
				m_IsResizing = false;
				m_NeedsUpdate = true;
				m_IsPaused = false;
				m_IsMinimized = false;
			}
			else if (m_IsMaximized)
			{
				m_IsResizing = false;
				m_NeedsUpdate = true;
				m_IsPaused = false;
				m_IsMaximized = false;
			}
		}
	}
	return 0;

	case WM_ENTERSIZEMOVE:
		m_IsPaused = true;
		m_IsResizing = true;
		return 0;

	case WM_EXITSIZEMOVE:
		m_IsResizing = false;
		m_NeedsUpdate = true;
		m_IsPaused = false;
		return 0;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 400;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 400;
		return 0;

	case WM_CLOSE:
		m_IsRunning = false;
		return 0;

	default:
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
}


#endif
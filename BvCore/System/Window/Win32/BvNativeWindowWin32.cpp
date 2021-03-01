#include "BvCore/System/Window/Win32/BvNativeWindowWin32.h"
#include "BvCore/Utils/BvDebug.h"
#include "BvCore/System/Threading/BvThread.h"


constexpr uint32_t kCreateWindowMessage = WM_APP + 0x100;
constexpr uint32_t kDestroyWindowMessage = kCreateWindowMessage + 1;


constexpr DWORD kWindowedStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
constexpr DWORD kWindowedExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
constexpr DWORD kFullscreenStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
constexpr DWORD kFullscreenExStyle = WS_EX_APPWINDOW;


class BvWindowsMessageController
{
	BV_NOCOPYMOVE(BvWindowsMessageController);

public:
	BvWindowsMessageController();
	~BvWindowsMessageController();

	void Start();
	void Stop();

	DWORD GetThreadId() const { return m_MessageThread.GetId(); }

private:
	BvThread m_MessageThread;
};


BvWindowsMessageController g_WindowsMessageController;


BvWindowsMessageController::BvWindowsMessageController()
	: m_MessageThread(BvThread([this]()
		{
			Start();
		}))
{
}


BvWindowsMessageController::~BvWindowsMessageController()
{
	Stop();
}


void BvWindowsMessageController::Start()
{
	RAWINPUTDEVICE device;
	device.usUsagePage = 0x01;
	device.usUsage = 0x06;
	device.dwFlags = RIDEV_NOLEGACY; // do not generate legacy messages such as WM_KEYDOWN
	device.hwndTarget = nullptr; // If no HWND is specified, WM_INPUT is triggered on the foreground window
	RegisterRawInputDevices(&device, 1, sizeof(device));

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		if (msg.message == kCreateWindowMessage)
		{
			BvNativeWindow* pWindow = reinterpret_cast<BvNativeWindow*>(msg.wParam);
			pWindow->RequestCreate(reinterpret_cast<const char* const>(msg.lParam));

			continue;
		}
		if (msg.message == kDestroyWindowMessage)
		{
			BvNativeWindow* pWindow = reinterpret_cast<BvNativeWindow*>(msg.wParam);
			pWindow->RequestDestroy();

			continue;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


void BvWindowsMessageController::Stop()
{
	PostThreadMessage(DWORD(m_MessageThread.GetId()), WM_QUIT, 0, 0);
	m_MessageThread.Wait();
}


BvNativeWindow::BvNativeWindow(const char* const pName, const WindowDesc& windowDesc)
	: m_WindowDesc(windowDesc), m_WindowedWidth(windowDesc.m_Width), m_WindowedHeight(windowDesc.m_Height),
	m_Event(false, false)
{
	Create(pName);
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

		m_IsActive = rhs.m_IsActive;
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


void BvNativeWindow::Create(const char* const pName)
{
	PostThreadMessage(g_WindowsMessageController.GetThreadId(), kCreateWindowMessage,
		reinterpret_cast<WPARAM>(this), reinterpret_cast<LPARAM>(pName));
	m_Event.Wait();
}


void BvNativeWindow::Destroy()
{
	PostThreadMessage(g_WindowsMessageController.GetThreadId(), kDestroyWindowMessage, reinterpret_cast<WPARAM>(this), 0);
	m_Event.Wait();
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
	if (m_WindowDesc.m_WindowMode != WindowMode::kWindowed)
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
	if (m_WindowDesc.m_WindowMode != WindowMode::kWindowed)
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
	m_IsActive = true;
}


void BvNativeWindow::Hide()
{
	BvAssertMsg(m_hWnd != nullptr, "Window Handle can't be null");
	m_IsActive = false;
	ShowWindow(m_hWnd, SW_HIDE);
}


void BvNativeWindow::SetTitle(const char* const pTitle)
{
	BvAssertMsg(m_hWnd != nullptr, "Window Handle can't be null");
	SetWindowText(m_hWnd, pTitle);
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
		windowDesc.m_Width = m_FullscreenWidth;
		windowDesc.m_Height = m_FullscreenHeight;
		windowDesc.m_PosX = 0;
		windowDesc.m_PosY = 0;

		style = kFullscreenStyle;
		exStyle = kFullscreenExStyle;

		break;
	case WindowMode::kFullscreen:
		BvAssertMsg(0, "Not implemented yet");
		break;
	}
}


void BvNativeWindow::NotifySizeChanged()
{
	BvScopedLock lock(m_SizeChangedCallbackLock);
	for (auto && sizeChangedCallback : m_SizeChangedCallbacks)
	{
		sizeChangedCallback.second(m_WindowDesc.m_Width, m_WindowDesc.m_Height);
	}
}

void BvNativeWindow::NotifyKeyboard(u32 vkCode, u32 scanCode, bool isKeyDown)
{
	BvScopedLock lock(m_KeyboardCallbackLock);
	for (auto&& keyboardCallback : m_KeyboardCallbacks)
	{
		keyboardCallback.second(vkCode, scanCode, isKeyDown);
	}
}


void BvNativeWindow::RequestCreate(const char* const pName)
{
	{
		//DEVMODE devMode = {};
		//devMode.dmSize = sizeof(DEVMODE);
		//if (!EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode))
		//{
		//	BV_WIN32_ERROR();
		//}
		//SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
		m_FullscreenWidth = GetSystemMetrics(SM_CXSCREEN);//devMode.dmPelsWidth;
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
		wndClass.lpszClassName = pName;
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

	m_hWnd = CreateWindowEx(exStyle, pName, pName, style,
		modeDesc.m_PosX, modeDesc.m_PosY, modeDesc.m_Width, modeDesc.m_Height, nullptr, nullptr, hInstance, this);
	if (!m_hWnd)
	{
		BV_WIN32_ERROR();
	}

	SetTitle(pName);

	m_Event.Set();
}


void BvNativeWindow::RequestDestroy()
{
	if (m_hWnd)
	{
		constexpr auto kMaxClassNameSize = 256;
		char className[kMaxClassNameSize];
		GetClassName(m_hWnd, className, kMaxClassNameSize);

		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
		UnregisterClass(className, GetModuleHandle(nullptr));

		m_SizeChangedCallbacks.Clear();
	}

	m_Event.Set();
}


LRESULT BvNativeWindow::StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BvNativeWindow* pWindow = nullptr;

	if (uMsg == WM_NCCREATE)
	{
		pWindow = reinterpret_cast<BvNativeWindow*>((reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		pWindow->m_hWnd = hwnd;

		return TRUE;
	}
	else
	{
		pWindow = reinterpret_cast<BvNativeWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		return pWindow->WndProc(hwnd, uMsg, wParam, lParam);
	}
}


LRESULT BvNativeWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
		m_IsActive = LOWORD(wParam) != WA_INACTIVE;
		return 0;

	case WM_SIZE:
	{
		m_WindowDesc.m_Width = LOWORD(lParam);
		m_WindowDesc.m_Height = HIWORD(lParam);

		if (wParam == SIZE_MINIMIZED)
		{
			m_IsActive = false;
			m_IsMinimized = true;
			m_IsMaximized = false;
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			m_IsActive = true;
			m_IsResizing = false;
			m_IsMinimized = false;
			m_IsMaximized = true;

			NotifySizeChanged();
		}
		else if (wParam == SIZE_RESTORED)
		{
			if (m_IsMinimized)
			{
				m_IsActive = true;
				m_IsResizing = false;
				m_IsMinimized = false;
				NotifySizeChanged();
			}
			else if (m_IsMaximized)
			{
				m_IsActive = true;
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
		m_IsActive = false;
		m_IsResizing = true;
		return 0;

	case WM_EXITSIZEMOVE:
		m_IsResizing = false;
		m_IsActive = true;
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
		m_IsActive = false;
		return 0;

	case WM_DESTROY:
		// Don't want to do a PostQuitMessage(0) here, since we may have multiple windows
		// Therefore, there's no need to end the message loop
		return 0;

	case WM_INPUT:
	{
		RAWINPUT raw{};
		{
			u32 size = sizeof(RAWINPUT);
			GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));
		}

		// extract keyboard raw input data
		if (raw.header.dwType == RIM_TYPEKEYBOARD)
		{
			const RAWKEYBOARD& rawKB = raw.data.keyboard;
			u32 vkCode = rawKB.VKey;
			u32 scanCode = rawKB.MakeCode;
			u32 flags = rawKB.Flags;

			if (vkCode == 255)
			{
				// discard "fake keys" which are part of an escaped sequence
				return 0;
			}
			else if (vkCode == VK_SHIFT)
			{
				// correct left-hand / right-hand SHIFT
				vkCode = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);

				// When holding shift while pressing something on the numpad with num lock on,
				// it'll report a shift press that won't map to either VK_LSHIFT or VK_RSHIFT,
				// so discard this fake press
				if (vkCode != VK_LSHIFT && vkCode != VK_RSHIFT)
				{
					return 0;
				}
			}
			else if (vkCode == VK_NUMLOCK)
			{
				// correct PAUSE/BREAK and NUM LOCK silliness, and set the extended bit
				scanCode = (MapVirtualKey(vkCode, MAPVK_VK_TO_VSC) | 0x100);
			}

			// e0 and e1 are escape sequences used for certain special keys, such as PRINT and PAUSE/BREAK.
			// see http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
			const bool isE0 = ((flags & RI_KEY_E0) != 0);
			const bool isE1 = ((flags & RI_KEY_E1) != 0);

			if (isE1)
			{
				// for escaped sequences, turn the virtual key into the correct scan code using MapVirtualKey.
				// however, MapVirtualKey is unable to map VK_PAUSE (this is a known bug), hence we map that by hand.
				if (vkCode == VK_PAUSE)
					scanCode = 0x45;
				else
					scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);
			}

			switch (vkCode)
			{
				// right-hand CONTROL and ALT have their e0 bit set
			case VK_CONTROL: vkCode = isE0 ? VK_RCONTROL : VK_LCONTROL; break;
			case VK_MENU: vkCode = isE0 ? VK_RMENU : VK_LMENU; break;
				// NUMPAD ENTER has its e0 bit set
			case VK_RETURN: if (isE0) vkCode = VK_SEPARATOR; break;
				// the standard INSERT, DELETE, HOME, END, PRIOR and NEXT keys will always have their e0 bit set, but the
				// corresponding keys on the NUMPAD will not.
			case VK_INSERT: if (!isE0) vkCode = VK_NUMPAD0; break;
			case VK_DELETE: if (!isE0) vkCode = VK_DECIMAL; break;
			case VK_HOME: if (!isE0) vkCode = VK_NUMPAD7; break;
			case VK_END: if (!isE0) vkCode = VK_NUMPAD1; break;
			case VK_PRIOR: if (!isE0) vkCode = VK_NUMPAD9; break;
			case VK_NEXT: if (!isE0) vkCode = VK_NUMPAD3; break;
				// the standard arrow keys will always have their e0 bit set, but the
				// corresponding keys on the NUMPAD will not.
			case VK_LEFT: if (!isE0) vkCode = VK_NUMPAD4; break;
			case VK_RIGHT: if (!isE0) vkCode = VK_NUMPAD6; break;
			case VK_UP: if (!isE0) vkCode = VK_NUMPAD8; break;
			case VK_DOWN: if (!isE0) vkCode = VK_NUMPAD2; break;
				// NUMPAD 5 doesn't have its e0 bit set
			case VK_CLEAR: if (!isE0) vkCode = VK_NUMPAD5; break;
			}

			// a key can either produce a "make" or "break" scancode. this is used to differentiate between down-presses and releases
			// see http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
			const bool isKeyUp = ((flags & RI_KEY_BREAK) != 0);

			// getting a human-readable string
			//u32 key = (scanCode << 16) | (isE0 << 24);
			//char buffer[512] = {};
			//GetKeyNameText((LONG)key, buffer, 512);

			NotifyKeyboard(vkCode, scanCode, !isKeyUp);
		}
		return 0;
	}

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}
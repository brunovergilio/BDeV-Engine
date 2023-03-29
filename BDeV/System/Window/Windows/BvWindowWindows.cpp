#include "BvWindowWindows.h"
#include "BDeV/System/Input/Windows/BvInputWindows.h"
#include "BDeV/System/Input/Windows/BvKeyboardWindows.h"
#include "BDeV/System/Input/Windows/BvMouseWindows.h"
#include "BDeV/System/Platform/Windows/BvPlatformWindows.h"
#include "BDeV/System/Debug/BvDebug.h"


BvWindowWindows::BvWindowWindows(const WindowDesc& windowDesc, WNDPROC wndProc)
	: BvWindow(), m_X(windowDesc.m_X), m_Y(windowDesc.m_Y), m_Width(windowDesc.m_Width),
	m_Height(windowDesc.m_Height), m_IsVisible(windowDesc.m_IsVisible)
{
	Create(windowDesc, wndProc);
}


BvWindowWindows::~BvWindowWindows()
{
	Destroy();
}


void BvWindowWindows::Resize(u32 width, u32 height)
{
	const LONG windowStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	if (windowStyle & WS_OVERLAPPED)
	{
		const LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE);
		const LONG exStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);

		// This adjusts a zero rect to give us the size of the border
		RECT BorderRect{};
		::AdjustWindowRectEx(&BorderRect, style, false, exStyle);

		// Inflate the window size by the OS border
		width += BorderRect.right - BorderRect.left;
		height += BorderRect.bottom - BorderRect.top;
	}

	::SetWindowPos(m_hWnd, nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
}


void BvWindowWindows::Move(i32 x, i32 y)
{
	const LONG windowStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	if (windowStyle & WS_OVERLAPPED)
	{
		const LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE);
		const LONG exStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);

		// This adjusts a zero rect to give us the size of the border
		RECT BorderRect{};
		::AdjustWindowRectEx(&BorderRect, style, false, exStyle);

		// Border rect size is negative
		x += BorderRect.left;
		y += BorderRect.top;
	}

	::SetWindowPos(m_hWnd, nullptr, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}


void BvWindowWindows::MoveAndResize(i32 x, i32 y, u32 width, u32 height)
{
	const LONG windowStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	if (windowStyle & WS_OVERLAPPED)
	{
		const LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE);
		const LONG exStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);

		// This adjusts a zero rect to give us the size of the border
		RECT BorderRect{};
		::AdjustWindowRectEx(&BorderRect, style, false, exStyle);

		// Border rect size is negative - see MoveWindowTo
		x += BorderRect.left;
		y += BorderRect.top;

		// Inflate the window size by the OS border
		width += BorderRect.right - BorderRect.left;
		height += BorderRect.bottom - BorderRect.top;
	}

	::SetWindowPos(m_hWnd, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}


void BvWindowWindows::Minimize()
{
	if (!m_IsMinimized)
	{
		::ShowWindow(m_hWnd, SW_MINIMIZE);
	}
}


void BvWindowWindows::Maximize()
{
	if (!m_IsMaximized)
	{
		::ShowWindow(m_hWnd, SW_MAXIMIZE);
	}
}


void BvWindowWindows::Restore()
{
	if (m_IsMaximized || m_IsMinimized)
	{
		::ShowWindow(m_hWnd, SW_RESTORE);
	}
}


void BvWindowWindows::Show()
{
	if (!m_IsVisible)
	{
		m_IsVisible = true;
		::ShowWindow(m_hWnd, SW_SHOW);
	}
}


void BvWindowWindows::Hide()
{
	if (m_IsVisible)
	{
		m_IsVisible = false;
		::ShowWindow(m_hWnd, SW_HIDE);
	}
}


void BvWindowWindows::SetFocus()
{
	if (!m_HasFocus)
	{
		m_HasFocus = true;
		::SetFocus(m_hWnd);
	}
}


void BvWindowWindows::Flash()
{
	::FlashWindow(m_hWnd, FALSE);
}


void BvWindowWindows::DestroyOnClose(bool value)
{
	m_DestroyOnClose = value;
}


bool BvWindowWindows::IsMinimized() const
{
	return m_IsMinimized;
}


bool BvWindowWindows::IsMaximized() const
{
	return m_IsMaximized;
}


bool BvWindowWindows::IsVisible() const
{
	return m_IsVisible;
}


bool BvWindowWindows::HasFocus() const
{
	return m_HasFocus;
}


bool BvWindowWindows::IsValid() const
{
	return m_hWnd != nullptr;
}


bool BvWindowWindows::DestroyOnClose() const
{
	return m_DestroyOnClose;
}


LRESULT BvWindowWindows::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
		m_IsActive = LOWORD(wParam) != WA_INACTIVE;
		return 0;

	case WM_SETFOCUS:
		if (!m_HasFocus)
		{
			m_HasFocus = true;
			OnFocus(true);
		}
		return 0;

	case WM_KILLFOCUS:
		if (m_HasFocus)
		{
			m_HasFocus = false;
			OnFocus(false);
		}
		return 0;

	case WM_SIZE:
	{
		m_Width = LOWORD(lParam);
		m_Height = HIWORD(lParam);

		m_IsMoving = false;
		if (wParam == SIZE_MINIMIZED)
		{
			m_IsResizing = false;
			m_IsMinimized = true;
			m_IsMaximized = false;
			m_IsVisible = false;

			OnMinimize();
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			m_IsResizing = false;
			m_IsMinimized = false;
			m_IsMaximized = true;
			m_IsVisible = true;

			OnMaximize(m_Width, m_Height);
		}
		else if (wParam == SIZE_RESTORED)
		{
			m_IsVisible = true;
			if (m_IsMinimized)
			{
				m_IsResizing = false;
				m_IsMinimized = false;
			}
			else if (m_IsMaximized)
			{
				m_IsResizing = false;
				m_IsMaximized = false;
			}
			else
			{
				if (!m_IsResizing)
				{
					OnRestore(m_Width, m_Height);
				}
				else
				{
					OnResizing(m_Width, m_Height);
				}
			}
		}
	}
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
			auto pKeyboard = reinterpret_cast<BvKeyboardWindows*>(Input::GetInput()->GetKeyboard());
			pKeyboard->ProcessRawInputKeyboardMessage(raw.data.keyboard);
		}
		// extract mouse raw input data
		else if (raw.header.dwType == RIM_TYPEMOUSE)
		{
			auto pMouse = reinterpret_cast<BvMouseWindows*>(Input::GetInput()->GetMouse());
			pMouse->ProcessRawInputMouseMessage(raw.data.mouse);
		}
		return 0;
	}

	case WM_ENTERSIZEMOVE:
		m_IsResizing = true;
		m_IsMoving = true;
		return 0;

	case WM_EXITSIZEMOVE:
		m_IsResizing = false;
		m_IsMoving = false;
		OnResize(m_Width, m_Height);

		return 0;

	case WM_MOVE:
	{
		m_IsResizing = false;
		m_IsMoving = true;
		m_X = (i32)LOWORD(lParam);
		m_Y = (i32)HIWORD(lParam);

		OnMoving(m_X, m_Y);

		return 0;
	}

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_CLOSE:
		OnClose();

		if (m_DestroyOnClose)
		{
			m_IsVisible = false;
			Destroy();
		}
		else
		{
			Hide();
		}

		return 0;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}


void BvWindowWindows::Create(const WindowDesc& windowDesc, WNDPROC wndProc)
{
	i32 x = m_X;
	i32 y = m_Y;
	i32 width = (i32)m_Width;
	i32 height = (i32)m_Height;

	DWORD style = 0;
	DWORD exStyle = 0;

	if (!windowDesc.m_HasBorder)
	{
		exStyle |= WS_EX_WINDOWEDGE;
		style |= WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	}
	else
	{
		exStyle |= WS_EX_APPWINDOW;
		style |= WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;

		if (windowDesc.m_CanMaximize)
		{
			style |= WS_MAXIMIZEBOX;
		}
		if (windowDesc.m_CanMinimize)
		{
			style |= WS_MINIMIZEBOX;
		}

		if (windowDesc.m_CanResize)
		{
			style |= WS_THICKFRAME;
		}
		else
		{
			style |= WS_BORDER;
		}

		// X,Y, Width, Height defines the top-left pixel of the client area on the screen
		// This adjusts a zero rect to give us the size of the border
		RECT adjustedRect = { 0, 0, 0, 0 };
		AdjustWindowRectEx(&adjustedRect, style, false, exStyle);

		x += adjustedRect.left;
		y += adjustedRect.top;
		width += adjustedRect.right - adjustedRect.left;
		height += adjustedRect.bottom - adjustedRect.top;
	}

	m_hModule = 
#if defined(BV_STATIC_LIB)
	GetModuleHandleA(nullptr);
#else
	GetModuleHandleA("BDeV.dll");
#endif

	m_X = x;
	m_Y = y;

	m_hWnd = CreateWindowExA(exStyle, s_WindowClassName, windowDesc.m_Name ? windowDesc.m_Name : s_DefaultWindowName, style,
		x, y, width, height, nullptr, nullptr, m_hModule, this);
	if (!m_hWnd)
	{
		BV_OS_ERROR();
	}

	if (windowDesc.m_HasBorder && !windowDesc.m_CanClose)
	{
		EnableMenuItem(GetSystemMenu(m_hWnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if (m_IsVisible)
	{
		ShowWindow(m_hWnd, SW_SHOWNORMAL);
	}
}


void BvWindowWindows::Destroy()
{
	if (m_hWnd)
	{
		::DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
	}
}
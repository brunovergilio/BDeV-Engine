#include "BDeV/System/Window/BvWindow.h"
#include "BDeV/System/Platform/BvPlatform.h"
#include "BDeV/System/Debug/BvDebug.h"


BvWindow::BvWindow(const WindowDesc& windowDesc)
	: m_X(windowDesc.m_X), m_Y(windowDesc.m_Y), m_Width(windowDesc.m_Width),
	m_Height(windowDesc.m_Height), m_IsVisible(windowDesc.m_IsVisible)
{
	Create(windowDesc);
}


BvWindow::~BvWindow()
{
	Destroy();
}


void BvWindow::Resize(u32 width, u32 height)
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


void BvWindow::Move(i32 x, i32 y)
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


void BvWindow::Minimize()
{
	if (!m_IsMinimized)
	{
		::ShowWindow(m_hWnd, SW_MINIMIZE);
	}
}


void BvWindow::Maximize()
{
	if (!m_IsMaximized)
	{
		::ShowWindow(m_hWnd, SW_MAXIMIZE);
	}
}


void BvWindow::Restore()
{
	if (m_IsMaximized || m_IsMinimized)
	{
		::ShowWindow(m_hWnd, SW_RESTORE);
	}
}


void BvWindow::Show()
{
	if (!m_IsVisible)
	{
		m_IsVisible = true;
		::ShowWindow(m_hWnd, SW_SHOW);
	}
}


void BvWindow::Hide()
{
	if (m_IsVisible)
	{
		m_IsVisible = false;
		::ShowWindow(m_hWnd, SW_HIDE);
	}
}


void BvWindow::SetFocus()
{
	if (!m_HasFocus)
	{
		m_HasFocus = true;
		::SetFocus(m_hWnd);
	}
}


void BvWindow::Flash()
{
	::FlashWindow(m_hWnd, FALSE);
}


void BvWindow::DestroyOnClose(bool value)
{
	m_DestroyOnClose = value;
}


bool BvWindow::IsMinimized() const
{
	return m_IsMinimized;
}


bool BvWindow::IsMaximized() const
{
	return m_IsMaximized;
}


bool BvWindow::IsVisible() const
{
	return m_IsVisible;
}


bool BvWindow::HasFocus() const
{
	return m_HasFocus;
}


bool BvWindow::IsValid() const
{
	return m_hWnd != nullptr;
}


bool BvWindow::DestroyOnClose() const
{
	return m_DestroyOnClose;
}


HDC BvWindow::GetDC()
{
	if (!m_hDC)
	{
		m_hDC = ::GetDC(m_hWnd);
	}

	return m_hDC;
}


void BvWindow::Create(const WindowDesc& windowDesc)
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

	auto hModule = 
#if defined(BV_STATIC_LIB)
	GetModuleHandleA(nullptr);
#else
	GetModuleHandleA("BDeV.dll");
#endif

	m_X = x;
	m_Y = y;

	m_hWnd = CreateWindowExA(exStyle, BvPlatform::s_WindowClassName, windowDesc.m_Name ? windowDesc.m_Name : s_DefaultWindowName, style,
		x, y, width, height, nullptr, nullptr, hModule, this);
	if (!m_hWnd)
	{
		BvOSCrash();
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


void BvWindow::Destroy()
{
	if (m_hWnd)
	{
		if (m_hDC)
		{
			::ReleaseDC(m_hWnd, m_hDC);
			m_hDC = nullptr;
		}
		::DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
		m_IsVisible = false;
	}
}


void BvWindow::OnWindowEvent(const WindowEventData& eventData)
{
	switch (eventData.type)
	{
	case WindowEventType::kActivate:
		OnActivate(eventData.active);
		break;
	case WindowEventType::kSizeMoveBegin:
		OnSizeMoveBegin();
		break;
	case WindowEventType::kResize:
		m_Width = eventData.resizeData.width;
		m_Height = eventData.resizeData.height;
		m_CurrentState = eventData.resizeData.state;
		OnResize(m_Width, m_Height, m_CurrentState);
		break;
	case WindowEventType::kMove:
		m_X = eventData.moveData.x;
		m_Y = eventData.moveData.y;
		OnMove(m_X, m_Y);
		break;
	case WindowEventType::kSizeMoveEnd:
		OnSizeMoveEnd();
		break;
	case WindowEventType::kShow:
		m_IsVisible = true;
		OnShow();
		break;
	case WindowEventType::kHide:
		m_IsVisible = false;
		OnHide();
		break;
	case WindowEventType::kGotFocus:
		m_HasFocus = true;
		OnFocus(true);
		break;
	case WindowEventType::kLostFocus:
		m_HasFocus = false;
		OnFocus(false);
		break;
	case WindowEventType::kClose:
		OnClose();
		if (m_DestroyOnClose)
		{
			Destroy();
			BvPlatform::DestroyWindow(reinterpret_cast<BvWindow*>(this));
		}
		else
		{
			Hide();
		}
		break;
	default:
		break;
	}
}
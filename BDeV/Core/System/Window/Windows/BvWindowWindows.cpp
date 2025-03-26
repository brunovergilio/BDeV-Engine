#include "BDeV/Core/System/Window/Windows/BvWindowWindows.h"
#include "BDeV/Core/System/Application/BvApplication.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/Container/BvText.h"
#include "BDeV/Core/System/Memory/BvMemoryArea.h"
#include "BDeV/Core/System/Window/BvMonitor.h"
#include <dwmapi.h>


//#pragma comment(lib, "Dwmapi.lib")


BvWindow::BvWindow(BvApplication* pApplication, const WindowDesc& windowDesc)
	: m_pApplication(pApplication), m_WindowDesc(windowDesc), m_WindowMode(windowDesc.m_WindowMode)
{
	Create();
}


BvWindow::~BvWindow()
{
	Destroy();
}


void BvWindow::Reshape(i32 x, i32 y, u32 width, u32 height)
{
	if (m_WindowDesc.m_HasBorder)
	{
		const LONG style = ::GetWindowLongW(m_hWnd, GWL_STYLE);
		const LONG exStyle = ::GetWindowLongW(m_hWnd, GWL_EXSTYLE);

		// This adjusts a zero rect to give us the size of the border
		RECT BorderRect{};
		::AdjustWindowRectEx(&BorderRect, style, false, exStyle);

		// Border rect size is negative
		x += BorderRect.left;
		y += BorderRect.top;

		// Inflate the window size by the OS border
		width += BorderRect.right - BorderRect.left;
		height += BorderRect.bottom - BorderRect.top;
	}

	if (IsMaximized())
	{
		Restore();
	}

	::SetWindowPos(m_hWnd, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);

	OnSizeChanged(width, height);
}


void BvWindow::Resize(u32 width, u32 height)
{
	if (m_WindowMode != WindowMode::kWindowed)
	{
		return;
	}

	if (m_WindowDesc.m_HasBorder)
	{
		const LONG style = ::GetWindowLongW(m_hWnd, GWL_STYLE);
		const LONG exStyle = ::GetWindowLongW(m_hWnd, GWL_EXSTYLE);

		// This adjusts a zero rect to give us the size of the border
		RECT BorderRect{};
		::AdjustWindowRectEx(&BorderRect, style, false, exStyle);

		// Inflate the window size by the OS border
		width += BorderRect.right - BorderRect.left;
		height += BorderRect.bottom - BorderRect.top;
	}

	::SetWindowPos(m_hWnd, nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

	OnSizeChanged(width, height);
}


void BvWindow::Move(i32 x, i32 y)
{
	if (m_WindowMode != WindowMode::kWindowed)
	{
		return;
	}

	if (m_WindowDesc.m_HasBorder)
	{
		const LONG style = ::GetWindowLongW(m_hWnd, GWL_STYLE);
		const LONG exStyle = ::GetWindowLongW(m_hWnd, GWL_EXSTYLE);

		// This adjusts a zero rect to give us the size of the border
		RECT BorderRect{};
		::AdjustWindowRectEx(&BorderRect, style, false, exStyle);

		// Border rect size is negative
		x += BorderRect.left;
		y += BorderRect.top;
	}

	::SetWindowPos(m_hWnd, nullptr, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}


void BvWindow::SetWindowMode(WindowMode windowMode)
{
	if (m_WindowMode == windowMode)
	{
		return;
	}

	// Setup Win32 Flags to be used for Fullscreen mode
	LONG style = GetWindowLongW(m_hWnd, GWL_STYLE);
	const LONG fullscreenStyle = WS_POPUP;

	LONG windowedStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
	if (m_WindowDesc.m_HasBorder)
	{
		if (m_WindowDesc.m_CanMaximize)
		{
			windowedStyle |= WS_MAXIMIZEBOX;
		}

		if (m_WindowDesc.m_CanMinimize)
		{
			windowedStyle |= WS_MINIMIZEBOX;
		}

		if (m_WindowDesc.m_CanResize)
		{
			windowedStyle |= WS_THICKFRAME;
		}
		else
		{
			windowedStyle |= WS_BORDER;
		}
	}
	else
	{
		windowedStyle |= WS_POPUP | WS_BORDER;
	}

	if (windowMode == WindowMode::kWindowedFullscreen || windowMode == WindowMode::kFullscreen)
	{
		bool trueFullscreen = windowMode == WindowMode::kFullscreen;

		// Save windowed position
		if (m_WindowMode == WindowMode::kWindowed)
		{
			m_PreFullscreenPosition.length = sizeof(WINDOWPLACEMENT);
			::GetWindowPlacement(m_hWnd, &m_PreFullscreenPosition);
		}

		style &= ~windowedStyle;
		style |= fullscreenStyle;

		SetWindowLongW(m_hWnd, GWL_STYLE, style);
		::SetWindowPos(m_hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		if (!trueFullscreen)
		{
			// Ensure the window is restored if we are going for WindowedFullscreen
			ShowWindow(m_hWnd, SW_RESTORE);
		}

		const BvMonitor* pMonitor = trueFullscreen ? BvMonitor::Primary() : BvMonitor::FromWindow(this);
		const auto area = pMonitor->GetFullscreenArea();

		Reshape(area.m_Left, area.m_Top, area.m_Right - area.m_Left, area.m_Bottom - area.m_Top);
	}
	else
	{
		style &= ~fullscreenStyle;
		style |= windowedStyle;

		SetWindowLongW(m_hWnd, GWL_STYLE, style);
		::SetWindowPos(m_hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		// Save restored position
		if (m_PreFullscreenPosition.length)
		{
			::SetWindowPlacement(m_hWnd, &m_PreFullscreenPosition);
		}

		// Set the icon back again as it seems to get ignored if the application has ever started in full screen mode
		HICON HIcon = (HICON)::GetClassLongPtrW(m_hWnd, GCLP_HICON);
		if (HIcon != nullptr)
		{
			::SendMessageW(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)HIcon);
		}
	}
}


void BvWindow::Minimize()
{
	if (!m_FirstTimeShow)
	{
		::ShowWindow(m_hWnd, SW_MINIMIZE);
	}
	else
	{
		m_WindowDesc.m_WindowState = WindowState::kMinimized;
	}
}


void BvWindow::Maximize()
{
	if (!m_FirstTimeShow)
	{
		::ShowWindow(m_hWnd, SW_MAXIMIZE);
	}
	else
	{
		m_WindowDesc.m_WindowState = WindowState::kMaximized;
	}
}


void BvWindow::Restore()
{
	if (!m_FirstTimeShow)
	{
		::ShowWindow(m_hWnd, SW_RESTORE);
	}
	else
	{
		m_WindowDesc.m_WindowState = WindowState::kRestored;
	}
}


void BvWindow::Show()
{
	if (!m_IsVisible)
	{
		m_IsVisible = true;
		i32 cmdShow = SW_SHOW;
		if (m_FirstTimeShow)
		{
			switch (m_WindowDesc.m_WindowState)
			{
			case WindowState::kDefault:
			case WindowState::kRestored: cmdShow = SW_SHOWNORMAL; break;
			case WindowState::kMinimized: cmdShow = SW_MINIMIZE; break;
			case WindowState::kMaximized: cmdShow = SW_SHOWMAXIMIZED; break;
			}

			m_FirstTimeShow = false;
		}
		
		::ShowWindow(m_hWnd, cmdShow);
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
	if (!::GetFocus())
	{
		::SetFocus(m_hWnd);
	}
}


void BvWindow::Flash()
{
	::FlashWindow(m_hWnd, FALSE);
}


void BvWindow::SetText(const char* pText)
{
	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(pText, 0, nullptr, 0);
		wchar_t* pTextW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(pText, 0, pTextW, sizeNeeded);
		SetWindowTextW(m_hWnd, pTextW);
	}
}


bool BvWindow::IsMinimized() const
{
	return IsIconic(m_hWnd);
}


bool BvWindow::IsMaximized() const
{
	return IsZoomed(m_hWnd);
}


bool BvWindow::IsVisible() const
{
	return m_IsVisible;
}


bool BvWindow::HasFocus() const
{
	return ::GetFocus() == m_hWnd;
}


void BvWindow::OnSizeChanged(u32 width, u32 height)
{
	m_Width = width;
	m_Height = height;
}


void BvWindow::OnPosChanged(i32 x, i32 y)
{
	m_X = x;
	m_Y = y;
}


void BvWindow::OnClose()
{
	m_IsClosed = true;
}


void BvWindow::Create()
{
	i32 x = m_WindowDesc.m_X;
	i32 y = m_WindowDesc.m_Y;
	i32 width = (i32)m_WindowDesc.m_Width;
	i32 height = (i32)m_WindowDesc.m_Height;

	DWORD style = 0;
	DWORD exStyle = 0;

	constexpr DWORD fullscreenStyle = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	constexpr DWORD fullscreenExStyle = WS_EX_APPWINDOW;

	if (!m_WindowDesc.m_HasBorder)
	{
		exStyle |= WS_EX_WINDOWEDGE;
		style |= WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	}
	else
	{
		exStyle |= WS_EX_APPWINDOW;
		style |= WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;

		if (m_WindowDesc.m_CanMaximize)
		{
			style |= WS_MAXIMIZEBOX;
		}
		if (m_WindowDesc.m_CanMinimize)
		{
			style |= WS_MINIMIZEBOX;
		}

		if (m_WindowDesc.m_CanResize)
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

		x -= adjustedRect.left;
		y -= adjustedRect.top;
		width += adjustedRect.right - adjustedRect.left;
		height += adjustedRect.bottom - adjustedRect.top;
	}

	if (m_WindowMode == WindowMode::kWindowedFullscreen || m_WindowMode == WindowMode::kFullscreen)
	{
		bool trueFullscreen = m_WindowMode == WindowMode::kFullscreen;

		style = fullscreenStyle;
		exStyle = fullscreenExStyle;

		const BvMonitor* pMonitor = trueFullscreen ? BvMonitor::Primary() : BvMonitor::FromPoint(x, y);
		const auto area = pMonitor->GetFullscreenArea();
		x = area.m_Left;
		y = area.m_Top;
		width = area.m_Right - area.m_Left;
		height = area.m_Bottom - area.m_Top;
	}

	constexpr const auto pDefaultWindowName = L"BDeV Window";
	constexpr auto kMaxWindowNameSize = 128;
	wchar_t wideName[kMaxWindowNameSize];
	auto pChosenWindowName = pDefaultWindowName;
	if (m_WindowDesc.m_Name)
	{
		if (auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(m_WindowDesc.m_Name, 0, nullptr, 0))
		{
			BvTextUtilities::ConvertUTF8CharToWideChar(m_WindowDesc.m_Name, 0, wideName, sizeNeeded);
			pChosenWindowName = wideName;
		}
	}
	if (!m_hWnd)
	{
		m_hWnd = CreateWindowExW(exStyle, L"BDeVWindowClass", pChosenWindowName, style,
			x, y, width, height, nullptr, nullptr, GetModuleHandleW(nullptr), this);
		if (!m_hWnd)
		{
			return;
		}
	}
	else
	{
		SetWindowLongW(m_hWnd, GWL_STYLE, style);
		SetWindowLongW(m_hWnd, GWL_EXSTYLE, exStyle);
		SetWindowPos(m_hWnd, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		SetWindowTextW(m_hWnd, pChosenWindowName);
	}

	m_IsClosed = false;

	m_X = x;
	m_Y = y;
	m_Width = width;
	m_Height = height;

	if (m_WindowDesc.m_HasBorder && !m_WindowDesc.m_CanClose)
	{
		EnableMenuItem(GetSystemMenu(m_hWnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if (m_WindowDesc.m_IsVisible)
	{
		Show();
	}

	if (!m_WindowDesc.m_HasBorder)
	{
		DeleteMenu(GetSystemMenu(m_hWnd, false), SC_CLOSE, MF_BYCOMMAND);

		//const DWMNCRENDERINGPOLICY RenderingPolicy = DWMNCRP_DISABLED;
		//DwmSetWindowAttribute(m_hWnd, DWMWA_NCRENDERING_POLICY, &RenderingPolicy, sizeof(RenderingPolicy));

		//const BOOL bEnableNCPaint = false;
		//DwmSetWindowAttribute(m_hWnd, DWMWA_ALLOW_NCPAINT, &bEnableNCPaint, sizeof(bEnableNCPaint));
	}
}


void BvWindow::Destroy()
{
	if (m_hWnd)
	{
		::DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
		m_IsVisible = false;
	}
}
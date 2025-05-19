#include "BDeV/Core/System/Window/Windows/BvWindowWindows.h"
#include "BDeV/Core/System/Application/BvApplication.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/Container/BvText.h"
#include "BDeV/Core/System/Memory/BvMemoryArea.h"
#include "BDeV/Core/System/Window/BvMonitor.h"
//#include <dwmapi.h>


//#pragma comment(lib, "Dwmapi.lib")


constexpr DWORD kFullscreenStyle = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE;
constexpr DWORD kFullscreenExStyle = WS_EX_APPWINDOW/* | WS_EX_TOPMOST*/;


std::pair<DWORD, DWORD> GetWindowStyles(const WindowDesc& windowDesc)
{
	DWORD style = 0;
	DWORD exStyle = 0;

	if (!windowDesc.m_Fullscreen)
	{
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
		}
	}
	else
	{
		style = kFullscreenStyle;
		exStyle = kFullscreenExStyle;
	}

	return { style, exStyle };
}


void AdjustWindow(HWND hWnd, const WindowDesc& windowDesc, i32& x, i32& y, i32& width, i32& height)
{
	if (windowDesc.m_HasBorder)
	{
		auto [style, exStyle] = GetWindowStyles(windowDesc);

		RECT rect{};
		// This adjusts a zero rect to give us the size of the border
		::AdjustWindowRectEx(&rect, style, false, exStyle);

		// Border rect size is negative
		x += rect.left;
		y += rect.top;

		// Inflate the window size by the OS border
		width += rect.right - rect.left;
		height += rect.bottom - rect.top;
	}
}


void AdjustWindowSize(HWND hWnd, const WindowDesc& windowDesc, i32& width, i32& height)
{
	i32 x, y;
	AdjustWindow(hWnd, windowDesc, x, y, width, height);
}


void AdjustWindowPosition(HWND hWnd, const WindowDesc& windowDesc, i32& x, i32& y)
{
	i32 w, h;
	AdjustWindow(hWnd, windowDesc, x, y, w, h);
}


BvWindow::BvWindow(BvApplication* pApplication, const WindowDesc& windowDesc)
	: m_pApplication(pApplication), m_WindowDesc(windowDesc)
{
	Create();
}


BvWindow::~BvWindow()
{
	Destroy();
}


void BvWindow::ChangeWindow(const WindowDesc& windowDesc)
{
	auto [style, exStyle] = GetWindowStyles(windowDesc);

	i32 x = windowDesc.m_X, y = windowDesc.m_Y, width = windowDesc.m_Width, height = windowDesc.m_Height;
	if (windowDesc.m_Fullscreen)
	{
		if (!m_WindowDesc.m_Fullscreen)
		{
			::GetWindowPlacement(m_hWnd, &m_PreFullscreenPosition);
		}

		BvMonitor* pMonitor = windowDesc.m_pMonitor ? windowDesc.m_pMonitor : BvMonitor::FromPoint(windowDesc.m_X, windowDesc.m_Y);
		BV_ASSERT(pMonitor != nullptr, "Monitor can't be nullptr");
		auto& monitorArea = pMonitor->GetFullscreenArea();
		x = monitorArea.m_Left;
		y = monitorArea.m_Top;
		width = monitorArea.m_Right - monitorArea.m_Left;
		height = monitorArea.m_Bottom - monitorArea.m_Top;
	}
	else
	{
		AdjustWindow(m_hWnd, windowDesc, x, y, width, height);
	}

	m_WindowDesc = windowDesc;

	::SetWindowLongPtrW(m_hWnd, GWL_STYLE, style);
	::SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, exStyle);
	::SetWindowPos(m_hWnd, nullptr, x, y, width, height, SWP_NOZORDER | SWP_FRAMECHANGED);

	if (windowDesc.m_pText)
	{
		SetText(windowDesc.m_pText);
	}

	// Always reset system menu to default if menu options may have changed
	::GetSystemMenu(m_hWnd, TRUE); // TRUE restores original system menu

	// Re-apply modifications after reset
	HMENU hSysMenu = ::GetSystemMenu(m_hWnd, FALSE);

	if (!m_WindowDesc.m_HasBorder)
	{
		::DeleteMenu(hSysMenu, SC_CLOSE, MF_BYCOMMAND);
	}
	else if (!m_WindowDesc.m_CanClose)
	{
		::EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	else
	{
		::EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
	}
	::DrawMenuBar(m_hWnd);

	if (m_WindowDesc.m_IsVisible)
	{
		// Force a show/update
		m_IsVisible = false;
		m_FirstTimeShow = false;

		Show();
	}
}


void BvWindow::Reshape(i32 x, i32 y, u32 width, u32 height)
{
	if (m_WindowDesc.m_Fullscreen)
	{
		return;
	}

	i32 w = width, h = height;
	AdjustWindow(m_hWnd, m_WindowDesc, x, y, w, h);

	if (IsMaximized())
	{
		Restore();
	}

	::SetWindowPos(m_hWnd, nullptr, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}


void BvWindow::Resize(u32 width, u32 height)
{
	if (m_WindowDesc.m_Fullscreen)
	{
		return;
	}

	i32 w = width, h = height;
	AdjustWindowSize(m_hWnd, m_WindowDesc, w, h);

	::SetWindowPos(m_hWnd, nullptr, 0, 0, w, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
}


void BvWindow::Move(i32 x, i32 y)
{
	if (m_WindowDesc.m_Fullscreen)
	{
		return;
	}

	AdjustWindowPosition(m_hWnd, m_WindowDesc, x, y);

	::SetWindowPos(m_hWnd, nullptr, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}


void BvWindow::SetFullscreen(bool value, BvMonitor* pMonitor)
{
	if (!pMonitor)
	{
		pMonitor = m_WindowDesc.m_pMonitor;
		BV_ASSERT(pMonitor != nullptr, "Monitor can't be nullptr");
	}

	if (m_WindowDesc.m_Fullscreen == value && m_WindowDesc.m_pMonitor == pMonitor)
	{
		return;
	}

	auto newWindowDesc = m_WindowDesc;
	newWindowDesc.m_Fullscreen = value;
	newWindowDesc.m_pMonitor = pMonitor;
	if (!newWindowDesc.m_Fullscreen && m_WindowDesc.m_Fullscreen)
	{
		RECT rect{};
		auto [style, exStyle] = GetWindowStyles(newWindowDesc);
		::AdjustWindowRectEx(&rect, style, FALSE, exStyle);
		
		newWindowDesc.m_X = m_PreFullscreenPosition.rcNormalPosition.left - rect.left;
		newWindowDesc.m_Y = m_PreFullscreenPosition.rcNormalPosition.top - rect.top;
		newWindowDesc.m_Width = m_PreFullscreenPosition.rcNormalPosition.right - m_PreFullscreenPosition.rcNormalPosition.left - (rect.right - rect.left);
		newWindowDesc.m_Height = m_PreFullscreenPosition.rcNormalPosition.bottom - m_PreFullscreenPosition.rcNormalPosition.top - (rect.bottom - rect.top);
	}

	ChangeWindow(newWindowDesc);
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
		::UpdateWindow(m_hWnd);
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
	if (::GetFocus() != m_hWnd)
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
	auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(pText, 0, nullptr, 0);
	wchar_t* pTextW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
	BvTextUtilities::ConvertUTF8CharToWideChar(pText, 0, pTextW, sizeNeeded);
	SetWindowTextW(m_hWnd, pTextW);
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
	m_WindowDesc.m_Width = width;
	m_WindowDesc.m_Height = height;
}


void BvWindow::OnPosChanged(i32 x, i32 y)
{
	m_WindowDesc.m_X = x;
	m_WindowDesc.m_Y = y;
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

	auto [style, exStyle] = GetWindowStyles(m_WindowDesc);

	BvMonitor* pMonitor = m_WindowDesc.m_pMonitor ? m_WindowDesc.m_pMonitor : BvMonitor::FromPoint(m_WindowDesc.m_X, m_WindowDesc.m_Y);
	BV_ASSERT(pMonitor != nullptr, "Monitor can't be nullptr");

	auto& monitorArea = pMonitor->GetFullscreenArea();
	if (m_WindowDesc.m_Fullscreen)
	{
		x = monitorArea.m_Left;
		y = monitorArea.m_Top;
		width = monitorArea.m_Right - monitorArea.m_Left;
		height = monitorArea.m_Bottom - monitorArea.m_Top;
	}
	else
	{
		if (m_WindowDesc.m_HasBorder)
		{
			RECT rect{};
			// This adjusts a zero rect to give us the size of the border
			::AdjustWindowRectEx(&rect, style, false, exStyle);

			// Border rect size is negative
			x += rect.left;
			y += rect.top;

			// Inflate the window size by the OS border
			width += rect.right - rect.left;
			height += rect.bottom - rect.top;
		}

		if (m_WindowDesc.m_pMonitor)
		{
			x += monitorArea.m_Left;
			y += monitorArea.m_Top;
		}
	}
	m_WindowDesc.m_pMonitor = pMonitor;

	constexpr const auto pDefaultWindowName = L"BDeV Window";
	constexpr auto kMaxWindowNameSize = 128;
	wchar_t wideName[kMaxWindowNameSize];
	auto pChosenWindowName = pDefaultWindowName;
	if (m_WindowDesc.m_pText)
	{
		if (auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(m_WindowDesc.m_pText, 0, nullptr, 0))
		{
			BvTextUtilities::ConvertUTF8CharToWideChar(m_WindowDesc.m_pText, 0, wideName, sizeNeeded);
			pChosenWindowName = wideName;
		}
	}

	m_hWnd = CreateWindowExW(exStyle, L"BDeVWindowClass", pChosenWindowName, style,
		x, y, width, height, nullptr, nullptr, GetModuleHandleW(nullptr), this);
	BV_ASSERT(m_hWnd != nullptr, "Window Handle is nullptr - check its parameters");

	::GetWindowPlacement(m_hWnd, &m_PreFullscreenPosition);

	m_IsClosed = false;

	::GetSystemMenu(m_hWnd, TRUE);
	HMENU hSysMenu = ::GetSystemMenu(m_hWnd, FALSE);

	if (!m_WindowDesc.m_HasBorder)
	{
		::DeleteMenu(hSysMenu, SC_CLOSE, MF_BYCOMMAND);
		//const DWMNCRENDERINGPOLICY RenderingPolicy = DWMNCRP_DISABLED;
		//DwmSetWindowAttribute(m_hWnd, DWMWA_NCRENDERING_POLICY, &RenderingPolicy, sizeof(RenderingPolicy));

		//const BOOL bEnableNCPaint = false;
		//DwmSetWindowAttribute(m_hWnd, DWMWA_ALLOW_NCPAINT, &bEnableNCPaint, sizeof(bEnableNCPaint));
	}
	else if (!m_WindowDesc.m_CanClose)
	{
		::EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	else
	{
		::EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
	}
	::DrawMenuBar(m_hWnd);

	if (m_WindowDesc.m_IsVisible)
	{
		Show();
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
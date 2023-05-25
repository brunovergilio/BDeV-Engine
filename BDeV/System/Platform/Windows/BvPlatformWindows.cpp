#include "BDeV/System/Platform/BvPlatform.h"
#include "BDeV/System/Window/BvWindow.h"
#include "BDeV/System/HID/BvKeyboard.h"
#include "BDeV/System/HID/BvMouse.h"
#include "BDeV/Container/BvRobinMap.h"


BvRobinMap<HWND, BvWindow*> g_Windows;
BvVector<BvWindow*> g_WindowsToDelete;


void BvPlatform::Initialize()
{
	// Register the window class.
	WNDCLASSEXA wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEXA);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = BvPlatform::WndProc;
	//wndClass.cbClsExtra = 0;
	//wndClass.cbWndExtra = 0;
#if defined(BV_STATIC_LIB)
	wndClass.hInstance = GetModuleHandleA(nullptr);
#else
	wndClass.hInstance = GetModuleHandleA("BDeV.dll");
#endif
	wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	//wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = BvPlatform::s_WindowClassName;
	//wndClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

	RegisterClassExA(&wndClass);

	RAWINPUTDEVICE rawInputDevices[2]{};
	rawInputDevices[0].usUsagePage = 0x1;
	rawInputDevices[0].usUsage = 0x6;
	rawInputDevices[0].dwFlags = 0; // RIDEV_NOLEGACY; // do not generate legacy messages such as WM_KEYDOWN
	rawInputDevices[0].hwndTarget = nullptr; // If no HWND is specified, WM_INPUT is triggered on the foreground window
	
	rawInputDevices[1].usUsagePage = 0x1;
	rawInputDevices[1].usUsage = 0x2;
	rawInputDevices[1].dwFlags = 0;
	rawInputDevices[1].hwndTarget = nullptr; // If no HWND is specified, WM_INPUT is triggered on the foreground window
	
	RegisterRawInputDevices(rawInputDevices, 2, sizeof(RAWINPUTDEVICE));
}


void BvPlatform::Shutdown()
{
	RAWINPUTDEVICE rawInputDevices[2]{};
	rawInputDevices[0].usUsagePage = 0x1;
	rawInputDevices[0].usUsage = 0x6;
	rawInputDevices[0].dwFlags = RIDEV_REMOVE;
	rawInputDevices[0].hwndTarget = nullptr;
	
	rawInputDevices[1].usUsagePage = 0x1;
	rawInputDevices[1].usUsage = 0x2;
	rawInputDevices[1].dwFlags = RIDEV_REMOVE;
	rawInputDevices[1].hwndTarget = nullptr;
	
	RegisterRawInputDevices(rawInputDevices, 2, sizeof(RAWINPUTDEVICE));

	if (g_WindowsToDelete.Size() > 0)
	{
		for (auto pWindow : g_WindowsToDelete)
		{
			delete pWindow;
		}
		g_WindowsToDelete.Clear();
	}

	HMODULE hModule = 
#if defined(BV_STATIC_LIB)
	GetModuleHandleA(nullptr);
#else
	GetModuleHandleA("BDeV.dll");
#endif

	UnregisterClassA(BvPlatform::s_WindowClassName, hModule);
}


void BvPlatform::ProcessOSEvents()
{
	if (g_WindowsToDelete.Size() > 0)
	{
		for (auto pWindow : g_WindowsToDelete)
		{
			delete pWindow;
		}
		g_WindowsToDelete.Clear();
	}

	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


BvWindow* BvPlatform::CreateWindow(const WindowDesc& windowDesc)
{
	auto pWindowWindows = new BvWindow(windowDesc);
	g_Windows.Emplace(pWindowWindows->GetHandle(), pWindowWindows);

	return reinterpret_cast<BvWindow*>(pWindowWindows);
}


void BvPlatform::DestroyWindow(BvWindow* pWindow)
{
	auto pWindowWindows = reinterpret_cast<BvWindow*>(pWindow);
	g_WindowsToDelete.PushBack(pWindowWindows);
	g_Windows.Erase(pWindowWindows->GetHandle());
}


LRESULT BvPlatform::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BvWindow* pWindow = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		pWindow = reinterpret_cast<BvWindow*>((reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		pWindow->m_hWnd = hWnd;

		return TRUE;
	}
	
	pWindow = reinterpret_cast<BvWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	WindowEventData data;
	switch (uMsg)
	{
	case WM_ACTIVATE:
		data.type = WindowEventType::kActivate;
		data.active = LOWORD(wParam) != WA_INACTIVE;
		pWindow->OnWindowEvent(data);

		return 0;

	case WM_SETFOCUS:
		if (!pWindow->HasFocus())
		{
			data.type = WindowEventType::kGotFocus;
			pWindow->OnWindowEvent(data);
		}

		return 0;

	case WM_KILLFOCUS:
		if (pWindow->HasFocus())
		{
			data.type = WindowEventType::kLostFocus;
			pWindow->OnWindowEvent(data);
		}

		return 0;

	case WM_SIZE:
	{
		data.type = WindowEventType::kResize;
		data.resizeData.width = LOWORD(lParam);
		data.resizeData.height = HIWORD(lParam);

		if (wParam == SIZE_MINIMIZED)
		{
			data.resizeData.state = WindowState::kMinimized;
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			data.resizeData.state = WindowState::kMaximized;
		}
		else if (wParam == SIZE_RESTORED)
		{
			data.resizeData.state = WindowState::kRestored;
		}

		pWindow->OnWindowEvent(data);
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
			Input::GetKeyboard()->ProcessRawInputKeyboardMessage(raw.data.keyboard);
		}
		// extract mouse raw input data
		else if (raw.header.dwType == RIM_TYPEMOUSE)
		{
			Input::GetMouse()->ProcessRawInputMouseMessage(raw.data.mouse);
		}
		return 0;
	}

	case WM_ENTERSIZEMOVE:
		data.type = WindowEventType::kSizeMoveBegin;
		pWindow->OnWindowEvent(data);

		return 0;

	case WM_EXITSIZEMOVE:
		data.type = WindowEventType::kSizeMoveEnd;
		pWindow->OnWindowEvent(data);

		return 0;

	case WM_MOVE:
	{
		data.type = WindowEventType::kMove;
		data.moveData.x = (i32)LOWORD(lParam);
		data.moveData.y = (i32)HIWORD(lParam);
		pWindow->OnWindowEvent(data);

		return 0;
	}

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_CLOSE:
		data.type = WindowEventType::kClose;
		pWindow->OnWindowEvent(data);

		return 0;

	case WM_DESTROY:
		return 0;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}
#include "BvPlatformWindows.h"
#include "BDeV/System/Window/Windows/BvWindowWindows.h"


BvRobinMap<HWND, BvWindowWindows*> g_Windows;
BvVector<BvWindowWindows*> g_WindowsToDelete;


void BvPlatform::Initialize()
{
	// Register the window class.
	WNDCLASSEXA wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEXA);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = BvPlatformWindows::WndProc;
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
	wndClass.lpszClassName = BvWindowWindows::s_WindowClassName;
	//wndClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

	RegisterClassExA(&wndClass);

	//RAWINPUTDEVICE rawInputDevices[2]{};
	//rawInputDevices[0].usUsagePage = 0x1;
	//rawInputDevices[0].usUsage = 0x6;
	//rawInputDevices[0].dwFlags = 0; // RIDEV_NOLEGACY; // do not generate legacy messages such as WM_KEYDOWN
	//rawInputDevices[0].hwndTarget = nullptr; // If no HWND is specified, WM_INPUT is triggered on the foreground window
	//
	//rawInputDevices[1].usUsagePage = 0x1;
	//rawInputDevices[1].usUsage = 0x2;
	//rawInputDevices[1].dwFlags = 0;
	//rawInputDevices[1].hwndTarget = nullptr; // If no HWND is specified, WM_INPUT is triggered on the foreground window
	//
	//RegisterRawInputDevices(rawInputDevices, 2, sizeof(RAWINPUTDEVICE));
}


void BvPlatform::Shutdown()
{
	//RAWINPUTDEVICE rawInputDevices[2]{};
	//rawInputDevices[0].usUsagePage = 0x1;
	//rawInputDevices[0].usUsage = 0x6;
	//rawInputDevices[0].dwFlags = RIDEV_REMOVE;
	//rawInputDevices[0].hwndTarget = nullptr;
	//
	//rawInputDevices[1].usUsagePage = 0x1;
	//rawInputDevices[1].usUsage = 0x2;
	//rawInputDevices[1].dwFlags = RIDEV_REMOVE;
	//rawInputDevices[1].hwndTarget = nullptr;
	//
	//RegisterRawInputDevices(rawInputDevices, 2, sizeof(RAWINPUTDEVICE));

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

	UnregisterClassA(BvWindowWindows::s_WindowClassName, hModule);
}


void BvPlatform::Update()
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


class BvWindow* BvPlatform::CreateWindow(const WindowDesc& windowDesc)
{
	auto pWindowWindows = new BvWindowWindows(windowDesc, BvPlatformWindows::WndProc);
	g_Windows.Emplace(pWindowWindows->GetHandle(), pWindowWindows);

	return pWindowWindows;
}


void BvPlatform::DestroyWindow(BvWindow* pWindow)
{
	auto pWindowWindows = reinterpret_cast<BvWindowWindows*>(pWindow);
	g_WindowsToDelete.PushBack(pWindowWindows);
	g_Windows.Erase(pWindowWindows->GetHandle());
}


LRESULT BvPlatformWindows::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BvWindowWindows* pWindow = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		pWindow = reinterpret_cast<BvWindowWindows*>((reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		//pWindow->m_hWnd = hwnd;

		return TRUE;
	}
	else if (uMsg == WM_DESTROY)
	{
		return 0;
	}
	else
	{
		pWindow = reinterpret_cast<BvWindowWindows*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		return pWindow->WndProc(hwnd, uMsg, wParam, lParam);
	}
}
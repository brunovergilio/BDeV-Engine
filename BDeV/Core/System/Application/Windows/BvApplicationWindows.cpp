#include "BDeV/Core/System/Application/BvApplication.h"
#include "BDeV/Core/System/Window/BvWindow.h"
#include "BDeV/Core/System/HID/BvKeyboard.h"
#include "BDeV/Core/System/HID/BvMouse.h"
#include "BDeV/Core/Container/BvRobinMap.h"
#include "BDeV/Core/Container/BvRobinSet.h"
#include "BDeV/Core/System/Application/BvMessageHandler.h"


extern void UpdateInputBuffers();
extern void ProcessLegacyKeyboardMessage(WPARAM wParam, LPARAM lParam, const BvKeyboard::KeyState*& pKeyState);
extern void ProcessRawInputKeyboardMessage(const RAWKEYBOARD& rawKB, const BvKeyboard::KeyState*& pKeyState);
extern void ProcessRawInputMouseMessage(const RAWMOUSE& rawMouse);
extern void ProcessCharInputMessage(u32 codePoint, LPARAM lParam, bool isDeadKey, const BvKeyboard::KeyState*& pKeyState);


#define BvDispatchEvent(e, ...) do											\
{																			\
	for (auto pMessageHandler : GetInstance()->m_pImpl->m_MessageHandlers)	\
	{																		\
		pMessageHandler->On##e(__VA_ARGS__);								\
	}																		\
} while (false);


struct Pimpl
{
	BvRobinMap<HWND, BvWindow*> m_Windows;
	BvVector<BvWindow*> m_WindowsToDelete;
	BvRobinSet<BvApplicationMessageHandler*> m_MessageHandlers;
	bool m_KeyboardUsesRawInput = false;
	bool m_MouseUsesRawInput = false;
	u32 m_HighSurrogate = 0;
};


void BvApplication::Initialize()
{
	// Register the window class.
	WNDCLASSEXW wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEXW);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = BvApplication::WndProc;
	//wndClass.cbClsExtra = 0;
	//wndClass.cbWndExtra = 0;
	wndClass.hInstance = GetModuleHandleW(nullptr);
	wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	//wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = L"BDeVWindowClass";
	//wndClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

	RegisterClassExW(&wndClass);

	m_pImpl = BV_NEW(Pimpl)();
}


void BvApplication::RegisterRawInput(bool keyboard, bool mouse)
{
	if (!(keyboard || mouse))
	{
		return;
	}

	RAWINPUTDEVICE rawInputDevices[2]{};
	u32 deviceCount = 0;
	if (keyboard)
	{
		rawInputDevices[deviceCount].usUsagePage = 0x1;
		rawInputDevices[deviceCount].usUsage = 0x6;
		rawInputDevices[deviceCount].dwFlags = 0; // RIDEV_NOLEGACY; // do not generate legacy messages such as WM_KEYDOWN
		rawInputDevices[deviceCount].hwndTarget = nullptr; // If no HWND is specified, WM_INPUT is triggered on the foreground window

		deviceCount++;

		m_pImpl->m_KeyboardUsesRawInput = true;
	}

	if (mouse)
	{
		rawInputDevices[deviceCount].usUsagePage = 0x1;
		rawInputDevices[deviceCount].usUsage = 0x2;
		rawInputDevices[deviceCount].dwFlags = 0;
		rawInputDevices[deviceCount].hwndTarget = nullptr; // If no HWND is specified, WM_INPUT is triggered on the foreground window

		deviceCount++;

		m_pImpl->m_MouseUsesRawInput = true;
	}

	RegisterRawInputDevices(rawInputDevices, deviceCount, sizeof(RAWINPUTDEVICE));
}


void BvApplication::Shutdown()
{
	RAWINPUTDEVICE rawInputDevices[2]{};
	u32 deviceCount = 0;
	if (m_pImpl->m_KeyboardUsesRawInput)
	{
		rawInputDevices[deviceCount].usUsagePage = 0x1;
		rawInputDevices[deviceCount].usUsage = 0x6;
		rawInputDevices[deviceCount].dwFlags = RIDEV_REMOVE;
		rawInputDevices[deviceCount].hwndTarget = nullptr;

		deviceCount++;
	}

	if (m_pImpl->m_MouseUsesRawInput)
	{
		rawInputDevices[deviceCount].usUsagePage = 0x1;
		rawInputDevices[deviceCount].usUsage = 0x2;
		rawInputDevices[deviceCount].dwFlags = RIDEV_REMOVE;
		rawInputDevices[deviceCount].hwndTarget = nullptr;

		deviceCount++;
	}
	
	RegisterRawInputDevices(rawInputDevices, deviceCount, sizeof(RAWINPUTDEVICE));

	if (m_pImpl->m_WindowsToDelete.Size() > 0)
	{
		for (auto pWindow : m_pImpl->m_WindowsToDelete)
		{
			BV_DELETE(pWindow);
		}
		m_pImpl->m_WindowsToDelete.Clear();
	}

	if (m_pImpl->m_Windows.Size() > 0)
	{
		for (auto& windowData : m_pImpl->m_Windows)
		{
			BV_DELETE(windowData.second);
		}
	}

	HMODULE hModule = GetModuleHandleW(nullptr);

	UnregisterClassW(L"BDeVWindowClass", hModule);

	BV_DELETE(m_pImpl);
}


void BvApplication::ProcessOSEvents()
{
	UpdateInputBuffers();

	if (!m_pImpl->m_KeyboardUsesRawInput)
	{
		if (GetAsyncKeyState(VK_SNAPSHOT) & 0x8000)
		{
			const BvKeyboard::KeyState* pKeyState;
			ProcessLegacyKeyboardMessage(VK_SNAPSHOT, 0, pKeyState);
			BvDispatchEvent(KeyDown, pKeyState->m_Key, pKeyState->m_ScanCode);
		}
	}

	if (m_pImpl->m_WindowsToDelete.Size() > 0)
	{
		for (auto pWindow : m_pImpl->m_WindowsToDelete)
		{
			BV_DELETE(pWindow);
		}
		m_pImpl->m_WindowsToDelete.Clear();
	}

	MSG msg = {};
	while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}


BvWindow* BvApplication::CreateWindow(const WindowDesc& windowDesc)
{
	auto pWindow = BV_NEW(BvWindow)(windowDesc);
	m_pImpl->m_Windows.Emplace(pWindow->m_hWnd, pWindow);

	return pWindow;
}


void BvApplication::DestroyWindow(BvWindow* pWindow)
{
	m_pImpl->m_Windows.Erase(pWindow->m_hWnd);
	m_pImpl->m_WindowsToDelete.PushBack(pWindow);
}


void BvApplication::AddMessageHandler(BvApplicationMessageHandler* pMessageHandler)
{
	m_pImpl->m_MessageHandlers.Emplace(pMessageHandler);
}


void BvApplication::RemoveMessageHandler(BvApplicationMessageHandler* pMessageHandler)
{
	m_pImpl->m_MessageHandlers.Erase(pMessageHandler);
}


BvApplication* BvApplication::GetInstance()
{
	static BvApplication app;
	return &app;
}


BvApplication::BvApplication()
{
}


BvApplication::~BvApplication()
{
}


LRESULT BvApplication::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BvWindow* pWindow = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		pWindow = reinterpret_cast<BvWindow*>((reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		pWindow->m_hWnd = hWnd;

		return TRUE;
	}
	
	pWindow = reinterpret_cast<BvWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
	auto pApp = GetInstance();

	switch (uMsg)
	{
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
			const BvKeyboard::KeyState* pKeyState;
			ProcessRawInputKeyboardMessage(raw.data.keyboard, pKeyState);
			if (pKeyState->m_IsPressed)
			{
				BvDispatchEvent(KeyDown, pKeyState->m_Key, pKeyState->m_ScanCode);
			}
			else
			{
				BvDispatchEvent(KeyUp, pKeyState->m_Key, pKeyState->m_ScanCode);
			}
		}
		// extract mouse raw input data
		else if (raw.header.dwType == RIM_TYPEMOUSE)
		{
			ProcessRawInputMouseMessage(raw.data.mouse);
		}
		return 0;
	}

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	{
		if (pApp->m_pImpl->m_KeyboardUsesRawInput)
		{
			break;
		}

		const BvKeyboard::KeyState* pKeyState;
		ProcessLegacyKeyboardMessage(wParam, lParam, pKeyState);
		if (pKeyState->m_IsPressed)
		{
			BvDispatchEvent(KeyDown, pKeyState->m_Key, pKeyState->m_ScanCode);
		}
		else
		{
			BvDispatchEvent(KeyUp, pKeyState->m_Key, pKeyState->m_ScanCode);
		}
		return 0;
	}

	case WM_CHAR:
	case WM_SYSCHAR:
	case WM_DEADCHAR:
	case WM_SYSDEADCHAR:
	{
		if (IS_HIGH_SURROGATE(wParam))
		{
			pApp->m_pImpl->m_HighSurrogate = u32(wParam);
			return 0;
		}

		u32 codePoint;
		if (IS_SURROGATE_PAIR(pApp->m_pImpl->m_HighSurrogate, wParam))
		{
			codePoint = ((pApp->m_pImpl->m_HighSurrogate - HIGH_SURROGATE_START) << 10) + (u32(wParam) - LOW_SURROGATE_START) + 0x10000;
			pApp->m_pImpl->m_HighSurrogate = 0;
		}
		else
		{
			codePoint = u32(wParam);
		}

		bool isDeadChar = uMsg == WM_DEADCHAR || uMsg == WM_SYSDEADCHAR;
		const BvKeyboard::KeyState* pKeyState;
		ProcessCharInputMessage(codePoint, lParam, isDeadChar, pKeyState);
		BvDispatchEvent(KeyChar, pKeyState->m_CodePoint, pKeyState->m_IsDeadKey);
		return 0;
	}

	case WM_ACTIVATE:
	{
		WindowActivationType activationType = WindowActivationType::kDeactivate;

		if (LOWORD(wParam) & WA_ACTIVE)
		{
			activationType = WindowActivationType::kActivate;
		}
		else if (LOWORD(wParam) & WA_CLICKACTIVE)
		{
			activationType = WindowActivationType::kClickActivate;
		}

		BvDispatchEvent(Activate, pWindow, activationType);

		return 0;
	}

	case WM_SETFOCUS:
		BvDispatchEvent(Focus, pWindow, true);
		return 0;

	case WM_KILLFOCUS:
		BvDispatchEvent(Focus, pWindow, false);
		return 0;

	case WM_SIZE:
	{
		auto width = (i32)LOWORD(lParam);
		auto height = (i32)HIWORD(lParam);

		if (pWindow->GetWindowMode() != WindowMode::kWindowed)
		{
			pWindow->OnSizeChanged(width, height);
		}

		WindowState currState = WindowState::kDefault;
		if (wParam == SIZE_MINIMIZED)
		{
			currState = WindowState::kMinimized;
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			currState = WindowState::kMaximized;
		}
		else if (wParam == SIZE_RESTORED)
		{
			currState = WindowState::kRestored;
		}

		BvDispatchEvent(SizeChange, pWindow, width, height, currState);
	}
	return 0;

	// case WM_SIZING: // This could be used if we wanted to control how the window is resized, like for preserving aspect ratio
	// case WM_MOVING: // This could be used to block the window from going to a specific portion of the screen

	case WM_MOVE:
	{
		auto x = (i32)LOWORD(lParam);
		auto y = (i32)HIWORD(lParam);
		pWindow->OnPosChanged(x, y);

		BvDispatchEvent(PosChange, pWindow, x, y);

		return 0;
	}

	case WM_ENTERSIZEMOVE:
		BvDispatchEvent(StartSizeMove, pWindow);
		return 0;

	case WM_EXITSIZEMOVE:
		BvDispatchEvent(EndSizeMove, pWindow);
		return 0;

	case WM_GETMINMAXINFO:
	{
		if (pWindow)
		{
			const auto& windowDesc = pWindow->GetWindowDesc();
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = std::max(((MINMAXINFO*)lParam)->ptMinTrackSize.x, (LONG)windowDesc.m_MinWidth);
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = std::max(((MINMAXINFO*)lParam)->ptMinTrackSize.y, (LONG)windowDesc.m_MinHeight);
		}
		return 0;
	}

	case WM_CLOSE:
		pWindow->Hide();
		
		BvDispatchEvent(Close, pWindow);

		return 0;

	case WM_DESTROY:
		return 0;

	case WM_ERASEBKGND:
		return 1;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
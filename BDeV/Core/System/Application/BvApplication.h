#pragma once


#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/System/Window/BvWindow.h"
#include "BDeV/Core/System/HID/BvHIDCommon.h"


class BvApplicationMessageHandler;


class BvApplication
{
	BV_NOCOPYMOVE(BvApplication);

public:
	static constexpr const char* s_WindowClassName = "BDeVWindowClass";

	static void Initialize();
	static void RegisterInputs(BvInputDeviceType deviceTypes, BvInputCreateFlags createFlags = BvInputCreateFlags::kNone);
	static void Shutdown();

	static void ProcessOSEvents();

	static BvWindow* CreateWindow(const WindowDesc& windowDesc);
	static void DestroyWindow(BvWindow* pWindow);

	static void AddMessageHandler(BvApplicationMessageHandler* pMessageHandler);
	static void RemoveMessageHandler(BvApplicationMessageHandler* pMessageHandler);

private:
	BvApplication() {}
	~BvApplication() {}

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	static LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
};
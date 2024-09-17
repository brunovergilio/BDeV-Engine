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

	void Initialize();
	void RegisterRawInput(bool keyboard = true, bool mouse = true);
	void Shutdown();

	void ProcessOSEvents();

	BvWindow* CreateWindow(const WindowDesc& windowDesc);
	void DestroyWindow(BvWindow* pWindow);

	void AddMessageHandler(BvApplicationMessageHandler* pMessageHandler);
	void RemoveMessageHandler(BvApplicationMessageHandler* pMessageHandler);

	static BvApplication* GetInstance();

private:
	BvApplication();
	~BvApplication();

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	static LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

private:
	struct Pimpl* m_pImpl = nullptr;
};
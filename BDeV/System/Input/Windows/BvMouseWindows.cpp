#include "BvMouseWindows.h"
#include "BDeV/System/Debug/BvDebug.h"


struct MouseData
{
	u8 mouseButtonStates{};
	i64 relativeMousePositionX{};
	i64 relativeMousePositionY{};
	i64 mouseWheelDeltaX{};
	i64 mouseWheelDeltaY{};
};


MouseData g_PrevGlobalMouseState{};
MouseData g_CurrGlobalMouseState{};


BvMouseWindows::BvMouseWindows()
{
	RAWINPUTDEVICE rawInputDevice{};
	rawInputDevice.usUsagePage = 0x1;
	rawInputDevice.usUsage = 0x2;
	rawInputDevice.dwFlags = 0; // RIDEV_NOLEGACY; // do not generate legacy messages such as WM_KEYDOWN
	rawInputDevice.hwndTarget = nullptr; // If no HWND is specified, WM_INPUT is triggered on the foreground window

	if (!RegisterRawInputDevices(&rawInputDevice, 1, sizeof(RAWINPUTDEVICE)))
	{
		BV_OS_ERROR();
	}
}


BvMouseWindows::~BvMouseWindows()
{
	RAWINPUTDEVICE rawInputDevice{};
	rawInputDevice.usUsagePage = 0x1;
	rawInputDevice.usUsage = 0x2;
	rawInputDevice.dwFlags = RIDEV_REMOVE;
	rawInputDevice.hwndTarget = nullptr;

	if (!RegisterRawInputDevices(&rawInputDevice, 1, sizeof(RAWINPUTDEVICE)))
	{
		BV_OS_ERROR();
	}
}


bool BvMouseWindows::KeyWentDown(BvMouseButton button) const
{
	return (g_CurrGlobalMouseState.mouseButtonStates & u8(button)) != 0
		&& (g_PrevGlobalMouseState.mouseButtonStates & u8(button)) == 0;
}


bool BvMouseWindows::KeyWentUp(BvMouseButton button) const
{
	return (g_CurrGlobalMouseState.mouseButtonStates & u8(button)) == 0;
}


bool BvMouseWindows::KeyIsPressed(BvMouseButton button) const
{
	return (g_CurrGlobalMouseState.mouseButtonStates & u8(button)) != 0;
}


void SetGlobalMouseStateData(BvMouseButton mouseButtons, i32 x, i32 y, i32 wheelDeltaX, i32 wheelDeltaY)
{
	g_PrevGlobalMouseState = g_CurrGlobalMouseState;
	g_CurrGlobalMouseState.mouseButtonStates = u8(mouseButtons);
	g_CurrGlobalMouseState.relativeMousePositionX += x;
	g_CurrGlobalMouseState.relativeMousePositionY += y;
	g_CurrGlobalMouseState.mouseWheelDeltaX += wheelDeltaX;
	g_CurrGlobalMouseState.mouseWheelDeltaX += wheelDeltaY;
}


void BvMouseWindows::ProcessRawInputMouseMessage(const RAWMOUSE& rawMouse) const
{
	i32 x = rawMouse.lLastX;
	i32 y = rawMouse.lLastY;

	BvMouseButton mouseButtons = BvMouseButton::kNone;
	i32 wheelDeltaX = 0;
	i32 wheelDeltaY = 0;

	u16 flags = rawMouse.usButtonFlags;
	if (flags & RI_MOUSE_LEFT_BUTTON_DOWN) { mouseButtons |= BvMouseButton::kLeft; }
	if (flags & RI_MOUSE_RIGHT_BUTTON_DOWN) { mouseButtons |= BvMouseButton::kRight; }
	if (flags & RI_MOUSE_MIDDLE_BUTTON_DOWN) { mouseButtons |= BvMouseButton::kMiddle; }
	if (flags & RI_MOUSE_BUTTON_4_DOWN) { mouseButtons |= BvMouseButton::kButton4; }
	if (flags & RI_MOUSE_BUTTON_5_DOWN) { mouseButtons |= BvMouseButton::kButton5; }
	if (flags & RI_MOUSE_WHEEL) { mouseButtons |= BvMouseButton::kWheel; wheelDeltaY = i16(rawMouse.usButtonData); }
	if (flags & RI_MOUSE_HWHEEL) { mouseButtons |= BvMouseButton::kHWheel; wheelDeltaX = i16(rawMouse.usButtonData); }

	SetGlobalMouseStateData(mouseButtons, x, y, wheelDeltaX, wheelDeltaY);
}
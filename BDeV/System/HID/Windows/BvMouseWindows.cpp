#include "BDeV/System/HID/BvMouse.h"
#include "BDeV/System/Debug/BvDebug.h"


struct MouseData
{
	u8 mouseButtonStates{};
	i64 relativeMousePositionX{};
	i64 relativeMousePositionY{};
	i64 mouseWheelDeltaX{};
	i64 mouseWheelDeltaY{};
};


MouseData g_CurrGlobalMouseState{};
MouseData g_PrevGlobalMouseState{};


BvMouse::BvMouse()
{
}


BvMouse::~BvMouse()
{
}


bool BvMouse::KeyWentDown(BvMouseButton button) const
{
	return (g_CurrGlobalMouseState.mouseButtonStates & u8(button)) != 0
		&& (g_PrevGlobalMouseState.mouseButtonStates & u8(button)) == 0;
}


bool BvMouse::KeyWentUp(BvMouseButton button) const
{
	return (g_CurrGlobalMouseState.mouseButtonStates & u8(button)) == 0
		&& (g_PrevGlobalMouseState.mouseButtonStates & u8(button)) != 0;
}


bool BvMouse::KeyIsDown(BvMouseButton button) const
{
	return (g_CurrGlobalMouseState.mouseButtonStates & u8(button)) != 0;
}


bool BvMouse::KeyIsUp(BvMouseButton button) const
{
	return (g_CurrGlobalMouseState.mouseButtonStates & u8(button)) == 0;
}


std::pair<i32, i32> BvMouse::GetGlobalPosition() const
{
	POINT p;
	GetCursorPos(&p);

	return std::pair<i32, i32>(p.x, p.y);
}


std::pair<i64, i64> BvMouse::GetRelativePosition() const
{
	return std::pair<i64, i64>(g_CurrGlobalMouseState.relativeMousePositionX, g_CurrGlobalMouseState.relativeMousePositionY);
}


void BvMouse::ProcessRawInputMouseMessage(const RAWMOUSE& rawMouse) const
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
	if (flags & RI_MOUSE_WHEEL) { mouseButtons |= BvMouseButton::kWheel; wheelDeltaY = i32(rawMouse.usButtonData); }
	if (flags & RI_MOUSE_HWHEEL) { mouseButtons |= BvMouseButton::kHWheel; wheelDeltaX = i32(rawMouse.usButtonData); }

	SetGlobalMouseStateData(mouseButtons, x, y, wheelDeltaX, wheelDeltaY);
}


void BvMouse::SetGlobalMouseStateData(BvMouseButton mouseButtons, i32 x, i32 y, i32 wheelDeltaX, i32 wheelDeltaY) const
{
	g_PrevGlobalMouseState = g_CurrGlobalMouseState;
	g_CurrGlobalMouseState.mouseButtonStates = u8(mouseButtons);
	g_CurrGlobalMouseState.relativeMousePositionX += x;
	g_CurrGlobalMouseState.relativeMousePositionY += y;
	g_CurrGlobalMouseState.mouseWheelDeltaX += wheelDeltaX;
	g_CurrGlobalMouseState.mouseWheelDeltaX += wheelDeltaY;
}


BvMouse* Input::GetMouse()
{
	static BvMouse mouse;
	return &mouse;
}
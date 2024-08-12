#include "BDeV/Core/System/HID/BvMouse.h"
#include "BDeV/Core/System/Debug/BvDebug.h"
#include "BDeV/Core/System/Windows/BvWindowsHeader.h"


void SetGlobalMouseStateData(BvMouseButton mouseButtons, i32 x, i32 y, i32 wheelDeltaX, i32 wheelDeltaY);


BvMouse::MouseState g_CurrGlobalMouseState{};
BvMouse::MouseState g_PrevGlobalMouseState{};


BvMouse::BvMouse()
{
}


BvMouse::~BvMouse()
{
}


bool BvMouse::KeyWentDown(BvMouseButton button) const
{
	return EHasFlag(g_CurrGlobalMouseState.mouseButtonStates, button) && !EHasFlag(g_PrevGlobalMouseState.mouseButtonStates, button);
}


bool BvMouse::KeyWentUp(BvMouseButton button) const
{
	return !EHasFlag(g_CurrGlobalMouseState.mouseButtonStates, button) && EHasFlag(g_PrevGlobalMouseState.mouseButtonStates, button);
}


bool BvMouse::KeyIsDown(BvMouseButton button) const
{
	return EHasFlag(g_CurrGlobalMouseState.mouseButtonStates, button);
}


bool BvMouse::KeyIsUp(BvMouseButton button) const
{
	return !EHasFlag(g_CurrGlobalMouseState.mouseButtonStates, button);
}


const BvMouse::MouseState& BvMouse::GetMouseState() const
{
	return g_CurrGlobalMouseState;
}


std::pair<i32, i32> BvMouse::GetGlobalPosition() const
{
	POINT p;
	GetCursorPos(&p);
	return std::make_pair(p.x, p.y);
}


std::pair<i64, i64> BvMouse::GetRelativePosition() const
{
	return std::pair<i64, i64>(g_CurrGlobalMouseState.relativeMousePositionX, g_CurrGlobalMouseState.relativeMousePositionY);
}


void ProcessRawInputMouseMessage(const RAWMOUSE& rawMouse)
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


void SetGlobalMouseStateData(BvMouseButton mouseButtons, i32 x, i32 y, i32 wheelDeltaX, i32 wheelDeltaY)
{
	g_PrevGlobalMouseState = g_CurrGlobalMouseState;
	g_CurrGlobalMouseState.mouseButtonStates = mouseButtons;
	g_CurrGlobalMouseState.relativeMousePositionX += x;
	g_CurrGlobalMouseState.relativeMousePositionY += y;
	g_CurrGlobalMouseState.mouseWheelDeltaX += wheelDeltaX;
	g_CurrGlobalMouseState.mouseWheelDeltaY += wheelDeltaY;
}
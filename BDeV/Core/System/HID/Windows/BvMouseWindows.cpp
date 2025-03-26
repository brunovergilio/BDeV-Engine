#include "BDeV/Core/System/HID/BvMouse.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"
#include "BDeV/Core/System/Window/BvWindow.h"


void SetGlobalMouseStateData(BvMouseButton mouseButtons, i32 x, i32 y, i32 wheelDeltaX, i32 wheelDeltaY);


BvMouse::MouseState g_CurrGlobalMouseState{};
BvMouse::MouseState g_PrevGlobalMouseState{};
BvMouseButton g_CurrMouseButtonStates{};
BvMouseButton g_PrevMouseButtonStates{};

constexpr u32 kMaxMouseStateChangesPerFrame = 64;
BvMouse::MouseState g_MouseStateChanges[kMaxMouseStateChangesPerFrame]{};
u32 g_MouseStateChangeCount = 0;


void UpdateMouse()
{
	g_PrevMouseButtonStates = g_CurrMouseButtonStates;
	g_PrevGlobalMouseState = g_CurrGlobalMouseState;

	g_CurrMouseButtonStates &= ~(BvMouseButton::kWheel | BvMouseButton::kHWheel);
	g_CurrGlobalMouseState.mouseButtonStates &= ~(BvMouseButton::kWheel | BvMouseButton::kHWheel);
	g_CurrGlobalMouseState.mouseWheelDeltaX = 0.0f;
	g_CurrGlobalMouseState.mouseWheelDeltaY = 0.0f;
	g_CurrGlobalMouseState.relativeMousePositionX = 0;
	g_CurrGlobalMouseState.relativeMousePositionY = 0;

	g_MouseStateChangeCount = 0;
}


BvMouse::BvMouse()
{
}


BvMouse::~BvMouse()
{
}


bool BvMouse::KeyWentDown(BvMouseButton button) const
{
	return EHasFlag(g_CurrMouseButtonStates, button) && !EHasFlag(g_PrevMouseButtonStates, button);
}


bool BvMouse::KeyWentUp(BvMouseButton button) const
{
	return !EHasFlag(g_CurrMouseButtonStates, button) && EHasFlag(g_PrevMouseButtonStates, button);
}


bool BvMouse::KeyIsDown(BvMouseButton button) const
{
	return EHasFlag(g_CurrMouseButtonStates, button);
}


bool BvMouse::KeyIsUp(BvMouseButton button) const
{
	return !EHasFlag(g_CurrMouseButtonStates, button);
}


const BvMouse::MouseState& BvMouse::GetMouseState() const
{
	return g_CurrGlobalMouseState;
}


u32 BvMouse::GetMouseStateChanges(MouseState* pMouseStates) const
{
	if (pMouseStates)
	{
		for (auto i = 0u; i < g_MouseStateChangeCount; ++i)
		{
			pMouseStates[i] = g_MouseStateChanges[i];
		}
	}

	return g_MouseStateChangeCount;
}


std::pair<i32, i32> BvMouse::GetGlobalPosition() const
{
	POINT p;
	GetCursorPos(&p);
	return std::make_pair(p.x, p.y);
}


std::pair<i32, i32> BvMouse::GetPosition(BvWindow* pWindow) const
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient((HWND)pWindow->GetHandle(), &p);
	return std::make_pair(p.x, p.y);
}


void ProcessLegacyMouseMessage(u32 flags, i32 x, i32 y, i32 wheelDeltaV, i32 wheelDeltaH, const BvMouse::MouseState*& pMouseState)
{
	BV_ASSERT(false, "Not implemented yet");

	BvMouseButton mouseButtons = BvMouseButton::kNone;

	if (flags & MK_LBUTTON) { mouseButtons |= BvMouseButton::kLeft; }
	if (flags & MK_RBUTTON) { mouseButtons |= BvMouseButton::kRight; }
	if (flags & MK_MBUTTON) { mouseButtons |= BvMouseButton::kMiddle; }
	if (flags & MK_XBUTTON1) { mouseButtons |= BvMouseButton::kButton4; }
	if (flags & MK_XBUTTON2) { mouseButtons |= BvMouseButton::kButton5; }

	if (wheelDeltaV) { mouseButtons |= BvMouseButton::kWheel; }
	if (wheelDeltaH) { mouseButtons |= BvMouseButton::kHWheel; }
}


void ProcessRawInputMouseMessage(const RAWMOUSE& rawMouse)
{
	i32 x = rawMouse.lLastX;
	i32 y = rawMouse.lLastY;

	BvMouseButton downButtons = BvMouseButton::kNone;
	BvMouseButton upButtons = BvMouseButton::kNone;
	i32 wheelDeltaX = 0;
	i32 wheelDeltaY = 0;

	u16 flags = rawMouse.usButtonFlags;
	if (flags & RI_MOUSE_LEFT_BUTTON_DOWN) { downButtons |= BvMouseButton::kLeft; }
	if (flags & RI_MOUSE_RIGHT_BUTTON_DOWN) { downButtons |= BvMouseButton::kRight; }
	if (flags & RI_MOUSE_MIDDLE_BUTTON_DOWN) { downButtons |= BvMouseButton::kMiddle; }
	if (flags & RI_MOUSE_BUTTON_4_DOWN) { downButtons |= BvMouseButton::kButton4; }
	if (flags & RI_MOUSE_BUTTON_5_DOWN) { downButtons |= BvMouseButton::kButton5; }

	if (flags & RI_MOUSE_LEFT_BUTTON_UP) { upButtons |= BvMouseButton::kLeft; }
	if (flags & RI_MOUSE_RIGHT_BUTTON_UP) { upButtons |= BvMouseButton::kRight; }
	if (flags & RI_MOUSE_MIDDLE_BUTTON_UP) { upButtons |= BvMouseButton::kMiddle; }
	if (flags & RI_MOUSE_BUTTON_4_UP) { upButtons |= BvMouseButton::kButton4; }
	if (flags & RI_MOUSE_BUTTON_5_UP) { upButtons |= BvMouseButton::kButton5; }

	if (flags & RI_MOUSE_WHEEL) { downButtons |= BvMouseButton::kWheel; wheelDeltaY = i32(i16(rawMouse.usButtonData)); }
	if (flags & RI_MOUSE_HWHEEL) { downButtons |= BvMouseButton::kHWheel; wheelDeltaX = i32(i16(rawMouse.usButtonData)); }

	g_CurrMouseButtonStates |= downButtons;
	g_CurrMouseButtonStates &= (~upButtons);

	SetGlobalMouseStateData(g_CurrMouseButtonStates, x, y, wheelDeltaX, wheelDeltaY);
}


void SetGlobalMouseStateData(BvMouseButton mouseButtons, i32 x, i32 y, i32 wheelDeltaX, i32 wheelDeltaY)
{
	g_CurrGlobalMouseState.mouseButtonStates = mouseButtons;
	g_CurrGlobalMouseState.relativeMousePositionX += x;
	g_CurrGlobalMouseState.relativeMousePositionY += y;
	g_CurrGlobalMouseState.mouseWheelDeltaX += wheelDeltaX / f32(WHEEL_DELTA);
	g_CurrGlobalMouseState.mouseWheelDeltaY += wheelDeltaY / f32(WHEEL_DELTA);

	if (g_MouseStateChangeCount == kMaxMouseStateChangesPerFrame)
	{
		BV_ASSERT_ONCE(false, "Increase kMaxMouseStateChangesPerFrame");
		return;
	}
	auto& currStateChange = g_MouseStateChanges[g_MouseStateChangeCount++];
	currStateChange.mouseButtonStates = mouseButtons;
	currStateChange.relativeMousePositionX = x;
	currStateChange.relativeMousePositionY = y;
	currStateChange.mouseWheelDeltaX = wheelDeltaX;
	currStateChange.mouseWheelDeltaY = wheelDeltaY;
}
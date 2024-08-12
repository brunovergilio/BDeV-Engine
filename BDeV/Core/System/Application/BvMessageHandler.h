#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Window/BvWindowCommon.h"
#include "BDeV/Core/System/HID/BvHIDCommon.h"


class BvWindow;


class BvApplicationMessageHandler
{
public:
	BvApplicationMessageHandler() {}
	virtual ~BvApplicationMessageHandler() = 0 {};

	// Input messages
	virtual void OnKeyDown(BvKey key, u32 scanCode) {}
	virtual void OnKeyUp(BvKey key, u32 scanCode) {}
	virtual void OnKeyChar(u32 codePoint, bool isDeadChar) {}
	virtual void OnKeyStateChange(const BvKeyboard::KeyState& keyState) {}
	virtual void OnMouseMove(i32 x, i32 y) {}

	// Window messages
	virtual void OnActivate(BvWindow* pWindow, WindowActivationType activationType) {}
	virtual void OnAppActivate(BvWindow* pWindow, bool active) {}
	virtual void OnFocus(BvWindow* pWindow, bool gotFocus) {}
	virtual void OnSizeChange(BvWindow* pWindow, u32 width, u32 height, WindowState windowState) {}
	virtual void OnPosChange(BvWindow* pWindow, i32 x, i32 y) {}
	virtual void OnStartSizeMove(BvWindow* pWindow) {}
	virtual void OnEndSizeMove(BvWindow* pWindow) {}
	virtual void OnClose(BvWindow* pWindow) {}
};
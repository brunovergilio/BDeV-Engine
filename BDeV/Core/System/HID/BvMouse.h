#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/HID/BvHIDCommon.h"
#include <utility>


class BvMouse final
{
	BV_NOCOPYMOVE(BvMouse);

public:
	struct MouseState
	{
		BvMouseButton mouseButtonStates{};
		i64 relativeMousePositionX{};
		i64 relativeMousePositionY{};
		i64 mouseWheelDeltaX{};
		i64 mouseWheelDeltaY{};
	};

	BvMouse();
	~BvMouse();

	// Checks if the key was just pressed
	bool KeyWentDown(BvMouseButton button) const;

	// Checks if the key was just released
	bool KeyWentUp(BvMouseButton button) const;

	// Checks if the key is down
	bool KeyIsDown(BvMouseButton button) const;

	// Checks if the key is up
	bool KeyIsUp(BvMouseButton button) const;

	// Retrieves the mouse's current state
	const MouseState& GetMouseState() const;

	// Retrieves all mouse state changes from the last message loop
	u32 GetMouseStateChanges(MouseState* pMouseStates = nullptr) const;

	// Retrieves the mouse's current global position
	std::pair<i32, i32> GetGlobalPosition() const;
};
#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/System/HID/BvHIDCommon.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Utils/BvEvent.h"

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include <Windows.h>
#endif


class BV_API BvMouse final
{
	BV_NOCOPYMOVE(BvMouse);

public:
	friend class BvPlatform;

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

	std::pair<i32, i32> GetGlobalPosition() const;
	std::pair<i64, i64> GetRelativePosition() const;

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	// Used for processing Windows RawInput Messages
	void ProcessRawInputMouseMessage(const RAWMOUSE& rawMouse) const;
	void SetGlobalMouseStateData(BvMouseButton mouseButtons, i32 x, i32 y, i32 wheelDeltaX, i32 wheelDeltaY) const;
#endif
};


namespace Input
{
	BV_API BvMouse* GetMouse();
}
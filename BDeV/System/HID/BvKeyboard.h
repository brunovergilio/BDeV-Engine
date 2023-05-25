#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/System/HID/BvHIDCommon.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Utils/BvEvent.h"

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include <Windows.h>
#endif


class BV_API BvKeyboard final
{
	BV_NOCOPYMOVE(BvKeyboard);

public:
	friend class BvPlatform;

	BvKeyboard();
	~BvKeyboard();

	// Checks if the key was just pressed
	bool KeyWentDown(BvKey key) const;

	// Checks if the key was just released
	bool KeyWentUp(BvKey key) const;

	// Checks if the key is down
	bool KeyIsDown(BvKey key) const;

	// Checks if the key is up
	bool KeyIsUp(BvKey key) const;

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	// Used for processing Windows RawInput Messages
	void ProcessRawInputKeyboardMessage(const RAWKEYBOARD& rawKB) const;
	void SetGlobalKeyboardStateData(BvKey key, u32 scanCode, bool isDown) const;
#endif

public:
	BvEvent<BvKey, u32> OnKeyWentDown;
	BvEvent<BvKey, u32> OnKeyWentUp;
	BvEvent<BvKey, u32> OnKeyDown;
};


namespace Input
{
	BV_API BvKeyboard* GetKeyboard();
}
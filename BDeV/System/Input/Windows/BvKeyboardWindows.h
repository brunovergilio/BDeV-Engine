#pragma once


#include "BDeV/System/Input/BvKeyboard.h"
#include "BDeV/Utils/BvUtils.h"
#include <Windows.h>


class BvKeyboardWindows final : public BvKeyboard
{
	BV_NOCOPYMOVE(BvKeyboardWindows);

public:
	BvKeyboardWindows();
	~BvKeyboardWindows();

	// Checks if the key was just pressed down
	bool KeyWentDown(BvKey key) const override;

	// Checks if the key was released
	bool KeyWentUp(BvKey key) const override;

	// Checks if the key is pressed down
	bool KeyIsPressed(BvKey key) const override;

	// Used for processing Windows RawInput Messages
	void ProcessRawInputKeyboardMessage(const RAWKEYBOARD& rawKB) const;

private:
	void SetGlobalKeyboardStateData(BvKey key, u32 scanCode, bool isDown) const;
};
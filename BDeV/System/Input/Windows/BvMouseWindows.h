#pragma once


#include "BDeV/System/Input/BvMouse.h"
#include "BDeV/Utils/BvUtils.h"
#include <Windows.h>


class BvMouseWindows final : public BvMouse
{
	BV_NOCOPYMOVE(BvMouseWindows);

public:
	BvMouseWindows();
	~BvMouseWindows();

	// Checks if the key was just pressed down
	bool KeyWentDown(BvMouseButton button) const override;

	// Checks if the key was released
	bool KeyWentUp(BvMouseButton button) const override;

	// Checks if the key is pressed down
	bool KeyIsPressed(BvMouseButton button) const override;

	// Used for processing Windows RawInput Messages
	void ProcessRawInputMouseMessage(const RAWMOUSE& rawMouse) const;
};
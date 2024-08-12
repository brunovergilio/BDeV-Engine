#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/HID/BvHIDCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"


class BvKeyboard final
{
	BV_NOCOPYMOVE(BvKeyboard);

public:
	static constexpr u32 kMaxKeyCount = 256;

	friend class BvInput;

	struct KeyState
	{
		u32 m_ScanCode;
		u32 m_CodePoint;
		BvKey m_Key;
		bool m_IsDeadKey;
		bool m_IsPressed;
	};

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

	const KeyState& GetKeyState(BvKey key) const;
};
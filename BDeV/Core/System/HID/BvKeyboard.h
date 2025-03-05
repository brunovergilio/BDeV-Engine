#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/HID/BvHIDCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"


class BvKeyboard final
{
	BV_NOCOPYMOVE(BvKeyboard);

public:
	static constexpr u32 kMaxKeyCount = 256;

	struct KeyState
	{
		u32 m_ScanCode;
		u32 m_CodePoint;
		BvKey m_Key;
		bool m_IsDeadKey;
		bool m_IsPressed;
	};

	struct CharInput
	{
		u32 m_CodePoint;
		bool m_IsDeadChar;
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

	// Retrieves a key's current state
	const KeyState& GetKeyState(BvKey key) const;

	// Retrieves all key state changes from the last message loop
	u32 GetKeyStateChanges(KeyState* pKeyStates = nullptr) const;

	// Retrieves all char inputs from the last message loop
	u32 GetCharInputs(CharInput* pCharInputs = nullptr) const;
};
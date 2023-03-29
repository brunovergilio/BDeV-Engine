#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/System/Input/BvInputCommon.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Utils/BvEvent.h"


class BV_API BvKeyboard
{
	BV_NOCOPYMOVE(BvKeyboard);

public:
	//// Gets the current keyboard state change
	//const BvKeyboardState* GetCurrentState() const;

	//// Gets the next keyboard state change from a base point
	//const BvKeyboardState* GetNextState(const BvKeyboardState* pBaseState) const;

	//// Gets the previous keyboard state change from a base point
	//const BvKeyboardState* GetPreviousState(const BvKeyboardState* pBaseState) const;

	// Checks if the key was just pressed down
	virtual bool KeyWentDown(BvKey key) const = 0;

	// Checks if the key was released
	virtual bool KeyWentUp(BvKey key) const = 0;

	// Checks if the key is pressed down
	virtual bool KeyIsPressed(BvKey key) const = 0;

protected:
	BvKeyboard() {}
	virtual ~BvKeyboard() {}

public:
	BvEvent<BvKey, u32> OnKeyDown;
	BvEvent<BvKey, u32> OnKeyPressed;
	BvEvent<BvKey, u32> OnKeyUp;
};
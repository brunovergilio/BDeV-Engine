#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/System/Input/BvInputCommon.h"
#include "BDeV/Utils/BvUtils.h"


class BV_API BvMouse
{
	BV_NOCOPYMOVE(BvMouse);

public:
	//// Gets the current mouse state change
	//const BvMouseState* GetCurrentMouseState() const;

	//// Gets the next mouse state change from a base point
	//const BvMouseState* GetNextMouseState(const BvMouseState* pBaseState) const;

	//// Gets the previous mouse state change from a base point
	//const BvMouseState* GetPreviousMouseState(const BvMouseState* pBaseState) const;

	// Checks if the key was just pressed down
	virtual bool KeyWentDown(BvMouseButton button) const = 0;

	// Checks if the key was released
	virtual bool KeyWentUp(BvMouseButton button) const = 0;

	// Checks if the key is pressed down
	virtual bool KeyIsPressed(BvMouseButton button) const = 0;

protected:
	BvMouse() {}
	virtual ~BvMouse() {}
};
#pragma once


#include <BDeV/BvCore.h>
#include "BDeV/Utils/BvEvent.h"
#include "BDeV/System/Input/BvInputCommon.h"


class BvKeyboard;
class BvMouse;


class BvInput
{
public:
	virtual BvKeyboard* GetKeyboard() = 0;
	virtual BvMouse* GetMouse() = 0;

protected:
	BvInput() {}
	virtual ~BvInput() {}
};


namespace Input
{
	BV_API BvInput* GetInput();
}
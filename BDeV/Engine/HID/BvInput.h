#pragma once


#include <BDeV/BvCore.h>
#include "BDeV/Utils/BvEvent.h"
#include "BDeV/System/HID/BvInputCommon.h"


class BvKeyboard;
class BvMouse;


class BvInput
{
	BV_NOCOPYMOVE(BvInput);

public:
	BvInput();
	~BvInput();

	BvKeyboard* GetKeyboard();
	BvMouse* GetMouse();

private:
	void Initialize();
	void Shutdown();

private:
	BvKeyboard* m_pKeyboard = nullptr;
	BvMouse* m_pMouse = nullptr;
};


namespace Input
{
	BvInput* GetInput();
}
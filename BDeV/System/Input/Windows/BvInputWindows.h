#pragma once


#include "BDeV/System/Input/BvInput.h"
#include "BvKeyboardWindows.h"
#include "BvMouseWindows.h"


class BvKeyboardWindows;
class BvMouseWindows;


class BvInputWindows : public BvInput
{
public:
	BvInputWindows();
	~BvInputWindows();

	void Initialize();
	void Shutdown();

	BV_INLINE BvKeyboard* GetKeyboard() override { return m_pKeyboard; }
	BV_INLINE BvMouse* GetMouse() override { return m_pMouse; }

private:
	BvKeyboardWindows* m_pKeyboard = nullptr;
	BvMouseWindows* m_pMouse = nullptr;
};
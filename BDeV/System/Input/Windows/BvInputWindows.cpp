#include "BvInputWindows.h"


BvInputWindows::BvInputWindows()
{
	Initialize();
}


BvInputWindows::~BvInputWindows()
{
	Shutdown();
}


void BvInputWindows::Initialize()
{
	m_pKeyboard = new BvKeyboardWindows();
	m_pMouse = new BvMouseWindows();
}


void BvInputWindows::Shutdown()
{
	if (m_pKeyboard)
	{
		delete m_pKeyboard;
		m_pKeyboard = nullptr;
	}

	if (m_pMouse)
	{
		delete m_pMouse;
		m_pMouse = nullptr;
	}
}


namespace Input
{
	BvInput* GetInput()
	{
		static BvInputWindows input;
		return &input;
	}
}
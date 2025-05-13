#pragma once


#include "BDeV/Core/BvCore.h"


enum class WindowMode : u8
{
	kWindowed,
	kFullscreen
};


enum class WindowState : u8
{
	kDefault,
	kRestored,
	kMinimized,
	kMaximized,
};


enum class WindowActivationType : u8
{
	kActivate,
	kClickActivate,
	kDeactivate
};


struct WindowDesc
{
	const char* m_pText = nullptr;
	u32 m_Width = 800;
	u32 m_Height = 640;
	i32 m_X = 0;
	i32 m_Y = 0;
	u32 m_MinWidth = 200;
	u32 m_MinHeight = 200;
	bool m_CanMinimize = true;
	bool m_CanMaximize = true;
	bool m_CanClose = true;
	bool m_CanResize = true;
	bool m_HasBorder = true;
	bool m_IsVisible = true;
	//bool m_HasTransparency = true;
	bool m_Fullscreen = false;
	WindowState m_WindowState = WindowState::kDefault;
	class BvMonitor* m_pMonitor = nullptr;
};
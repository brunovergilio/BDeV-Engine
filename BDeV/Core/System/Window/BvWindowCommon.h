#pragma once


#include "BDeV/Core/BvCore.h"


enum class WindowMode : u8
{
	kWindowed,
	kWindowedFullscreen,
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
	const char* m_Name = nullptr;
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
	WindowMode m_WindowMode = WindowMode::kWindowed;
	WindowState m_WindowState = WindowState::kDefault;
};


enum class WindowEventType : u8
{
	kActivate,
	kSizeMoveBegin,
	kResize,
	kMove,
	kSizeMoveEnd,
	kShow,
	kHide,
	kGotFocus,
	kLostFocus,
	kClose,
};


struct WindowEventData
{
	WindowEventType type;
	union
	{
		struct
		{
			u32 width;
			u32 height;
			WindowState state;
		} resizeData;
		struct
		{
			i32 x;
			i32 y;
		} moveData;
		bool active;
	};
};
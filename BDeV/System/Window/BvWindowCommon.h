#pragma once


#include "BDeV/BvCore.h"


enum class WindowMode : u8
{
	kWindowed,
	kWindowedFullscreen,
	kFullscreen
};


struct WindowDesc
{
	const char* m_Name = nullptr;
	u32 m_Width = 800;
	u32 m_Height = 640;
	i32 m_X = 0;
	i32 m_Y = 0;
	i32 m_MonitorIndex = 0;
	bool m_CanMinimize = true;
	bool m_CanMaximize = true;
	bool m_CanClose = true;
	bool m_CanResize = true;
	bool m_HasBorder = true;
	bool m_IsVisible = true;
	WindowMode m_WindowMode = WindowMode::kWindowed;
};


enum class WindowState : u8
{
	kRestored,
	kMinimized,
	kMaximized,
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
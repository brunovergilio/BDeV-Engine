#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Utils/BvEvent.h"


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
	Restored,
	Minimized,
	Maximized,
};


struct WindowEventData
{
	enum class Type : u8
	{
		kResize,
		kMove,
		kResizing,
		kMoving,
		kStateChanged,
		kShow,
		kHide,
		kGotFocus,
		kLostFocus,
		kClose,
	};

	Type type;
	union
	{
		struct
		{
			u32 w, h;
		} m_ResizeData;
		struct
		{
			i32 x, y;
		} m_MoveData;
	};
};


class BV_API BvWindow
{
	BV_NOCOPYMOVE(BvWindow);

protected:
	BvWindow() {}
	virtual ~BvWindow() {}

public:
	virtual void Resize(u32 width, u32 height) {}
	virtual void Move(i32 x, i32 y) {}
	virtual void MoveAndResize(i32 x, i32 y, u32 width, u32 height) {}
	virtual void Minimize() {}
	virtual void Maximize() {}
	virtual void Restore() {}
	virtual void Show() {}
	virtual void Hide() {}
	virtual void SetFocus() {}
	virtual void Flash() {}
	virtual void DestroyOnClose(bool value) {}

	virtual bool IsMinimized() const { return false; }
	virtual bool IsMaximized() const { return false; }
	virtual bool IsVisible() const { return false; }
	virtual bool HasFocus() const { return false; }
	virtual bool IsValid() const { return false; }
	virtual bool DestroyOnClose() const { return true; }

	virtual i32 GetX() const { return 0; }
	virtual i32 GetY() const { return 0; }
	virtual u32 GetWidth() const { return 0; }
	virtual u32 GetHeight() const { return 0; }

public:
	BvEvent<u32, u32> OnResizing;
	BvEvent<u32, u32> OnResize;
	BvEvent<i32, i32> OnMoving;
	BvEvent<i32, i32> OnMove;
	BvEvent<> OnMinimize;
	BvEvent<u32, u32> OnMaximize;
	BvEvent<u32, u32> OnRestore;
	BvEvent<> OnClose;
	BvEvent<> OnShow;
	BvEvent<> OnHide;
	BvEvent<bool> OnFocus;
};
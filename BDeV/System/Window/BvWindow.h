#pragma once


#include "BDeV/BvCore.h"
#include "BDeV\System\Window\BvWindowCommon.h"
#include "BDeV\Utils\BvEvent.h"

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include <Windows.h>
#endif


class BV_API BvWindow final
{
public:
	static constexpr const char* const s_DefaultWindowName = "BDeV Window";

	friend class BvPlatform;

	explicit BvWindow(const WindowDesc& windowDesc);
	~BvWindow();

	void Resize(u32 width, u32 height);
	void Move(i32 x, i32 y);
	void Minimize();
	void Maximize();
	void Restore();
	void Show();
	void Hide();
	void SetFocus();
	void Flash();
	void DestroyOnClose(bool value);

	bool IsMinimized() const;
	bool IsMaximized() const;
	bool IsVisible() const;
	bool HasFocus() const;
	bool IsValid() const;
	bool DestroyOnClose() const;

	i32 GetX() const { return m_X; }
	i32 GetY() const { return m_Y; }
	u32 GetWidth() const { return m_Width; }
	u32 GetHeight() const { return m_Height; }

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	HWND GetHandle() { return m_hWnd; }
#endif

private:
	void Create(const WindowDesc& windowDesc);
	void Destroy();

	void OnWindowEvent(const WindowEventData& eventData);

public:
	BvEvent<bool> OnActivate;
	BvEvent<> OnSizeMoveBegin;
	BvEvent<> OnSizeMoveEnd;
	BvEvent<u32, u32, WindowState> OnResize;
	BvEvent<i32, i32> OnMove;
	BvEvent<> OnMinimize;
	BvEvent<u32, u32> OnMaximize;
	BvEvent<u32, u32> OnRestore;
	BvEvent<> OnClose;
	BvEvent<> OnShow;
	BvEvent<> OnHide;
	BvEvent<bool> OnFocus;

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	HWND m_hWnd = nullptr;
#endif
	u32 m_Width = 800;
	u32 m_Height = 640;
	i32 m_X = 0;
	i32 m_Y = 0;
	bool m_HasFocus = false;
	bool m_IsActive = false;
	bool m_IsResizing = false;
	bool m_IsMoving = false;
	bool m_IsMinimized = false;
	bool m_IsMaximized = false;
	bool m_DestroyOnClose = true;
	bool m_IsVisible = true;
	WindowState m_CurrentState = WindowState::kRestored;
};
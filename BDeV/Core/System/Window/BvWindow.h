#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Window/BvWindowCommon.h"
#include "BDeV/Core/Utils/BvEvent.h"
#include <atomic>

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include "BDeV/Core/System/Windows/BvWindowsHeader.h"
#endif


class BvWindow final
{
public:
	friend class BvApplication;

	explicit BvWindow(const WindowDesc& windowDesc);
	~BvWindow();

	void Reshape(i32 x, i32 y, u32 width, u32 height);
	void Resize(u32 width, u32 height);
	void Move(i32 x, i32 y);
	void SetWindowMode(WindowMode windowMode);
	void Minimize();
	void Maximize();
	void Restore();
	void Show();
	void Hide();
	void SetFocus();
	void Flash();
	void SetText(const char* pText);
	void GetDimensions(i32& x, i32& y, u32& width, u32& height);

	bool IsMinimized() const;
	bool IsMaximized() const;
	bool IsVisible() const;
	bool HasFocus() const;
	bool IsValid() const;

	i32 GetX() const { return m_X; }
	i32 GetY() const { return m_Y; }
	u32 GetWidth() const { return m_Width; }
	u32 GetHeight() const { return m_Height; }
	WindowMode GetWindowMode() const { return m_WindowMode; }
	const WindowDesc& GetWindowDesc() const { return m_WindowDesc; }

	void OnSizeChanged(u32 width, u32 height);
	void OnPosChanged(i32 x, i32 y);

	void* GetHandle() const;

private:
	void Create(bool createNew = true);
	void Destroy();

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	HWND m_hWnd = nullptr;
	WINDOWPLACEMENT m_PreFullscreenPosition{};
#endif
	WindowDesc m_WindowDesc;
	u32 m_Width = 800;
	u32 m_Height = 640;
	i32 m_X = 0;
	i32 m_Y = 0;
	WindowMode m_WindowMode = WindowMode::kWindowed;
	bool m_IsVisible = false;
	bool m_FirstTimeShow = true;
};
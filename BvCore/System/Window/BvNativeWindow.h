#pragma once


#include "BvCore/BvPlatform.h"
#include "BvCore/Container/BvRobinMap.h"
#include <functional>
#include "BvCore/Container/BvString.h"
#include "BvCore/Utils/BvUtils.h"


enum class WindowMode : u8
{
	kWindowed,
	kBordeless,
	kFullscreen
};


struct WindowDesc
{
	BvString m_Name{};
	u32 m_Width = 800;
	u32 m_Height = 600;
	i32 m_PosX = 0;
	i32 m_PosY = 0;
	WindowMode m_WindowMode = WindowMode::kWindowed;
};


class BvNativeWindow
{
	BV_NOCOPY(BvNativeWindow);

public:
	BvNativeWindow(const WindowDesc& windowDesc);
	BvNativeWindow(BvNativeWindow&& rhs) noexcept;
	BvNativeWindow& operator=(BvNativeWindow&& rhs) noexcept;
	~BvNativeWindow();

	void Create();
	void Destroy();

	void ChangeMode(const WindowMode windowMode);
	void Resize(const u32 width, const u32 height);
	void Move(const i32 posX, const i32 posY);

	void Show();
	void Hide();
	void SetTitle(const char* const pTitle);

	const u32 GetWidth() const { return m_WindowDesc.m_Width; }
	const u32 GetHeight() const { return m_WindowDesc.m_Height; }
	const i32 GetPosX() const { return m_WindowDesc.m_PosX; }
	const i32 GetPosY() const { return m_WindowDesc.m_PosY; }

	const bool IsPaused() const { return m_IsPaused; }
	const bool IsResizing() const { return m_IsResizing; }
	const bool IsMinimized() const { return m_IsMinimized; }
	const bool IsMaximized() const { return m_IsMaximized; }

	BV_INLINE void AddSizeChangedCallback(void* const pObj, const std::function<void(const u32, const u32)> & windowResizeFn)
	{
		m_SizeChangedCallbacks[pObj] = (windowResizeFn);
	}

	BV_INLINE void RemoveSizeChangedCallback(void* const pObj)
	{
		m_SizeChangedCallbacks.Erase(pObj);
	}

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	BV_INLINE HWND GetHandle() const { return m_hWnd; }

private:
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void GetWindowDescForMode(const WindowMode windowMode, WindowDesc& windowDesc, DWORD& style, DWORD& exStyle);
#endif

private:
	void NotifySizeChanged();

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	HWND m_hWnd = nullptr;
#endif

	BvRobinMap<void*, std::function<void(const u32, const u32)>> m_SizeChangedCallbacks;
	WindowDesc m_WindowDesc;
	
	u32 m_FullscreenWidth = 0;
	u32 m_FullscreenHeight = 0;

	u32 m_WindowedWidth = 0;
	u32 m_WindowedHeight = 0;
	i32 m_WindowedPosX = 0;
	i32 m_WindowedPosY = 0;

	bool m_IsPaused = false;
	bool m_IsResizing = false;
	bool m_IsMinimized = false;
	bool m_IsMaximized = false;
};
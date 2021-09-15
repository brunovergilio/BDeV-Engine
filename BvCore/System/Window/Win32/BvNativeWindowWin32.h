#pragma once


#include "BvCore/Container/BvRobinMap.h"
#include "BvCore/Container/BvString.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCore/System/Threading/BvSync.h"
#include <functional>
#include <Windows.h>


enum class WindowMode : u8
{
	kWindowed,
	kBordeless,
	kFullscreen
};


struct WindowDesc
{
	u32 m_Width = 800;
	u32 m_Height = 600;
	i32 m_PosX = 0;
	i32 m_PosY = 0;
	WindowMode m_WindowMode = WindowMode::kWindowed;
};


class BvNativeWindow
{
	BV_NOCOPYMOVE(BvNativeWindow);

	friend class BvWindowsMessageHandler;

public:
	//BvNativeWindow();
	BvNativeWindow(const char * const pName, const WindowDesc& windowDesc = WindowDesc());
	//BvNativeWindow(BvNativeWindow&& rhs) noexcept;
	//BvNativeWindow& operator=(BvNativeWindow&& rhs) noexcept;
	~BvNativeWindow();

	void Create(const char* const pName);
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

	const WindowDesc& GetWindowDesc() const { return m_WindowDesc; }

	const bool IsActive() const { return m_IsActive; }
	const bool IsResizing() const { return m_IsResizing; }
	const bool IsMinimized() const { return m_IsMinimized; }
	const bool IsMaximized() const { return m_IsMaximized; }

	BV_INLINE void AddSizeChangedCallback(void* const pObj, const std::function<void(u32, u32)>& windowResizeFn)
	{
		BvScopedLock lock(m_SizeChangedCallbackLock);
		m_SizeChangedCallbacks[pObj] = (windowResizeFn);
	}

	BV_INLINE void RemoveSizeChangedCallback(void* const pObj)
	{
		BvScopedLock lock(m_SizeChangedCallbackLock);
		m_SizeChangedCallbacks.Erase(pObj);
	}

	BV_INLINE void AddKeyboardCallback(void* const pObj, const std::function<void(u32, u32, bool)>& keyboardFn)
	{
		BvScopedLock lock(m_KeyboardCallbackLock);
		m_KeyboardCallbacks[pObj] = (keyboardFn);
	}

	BV_INLINE void RemoveKeyboardCallback(void* const pObj)
	{
		BvScopedLock lock(m_KeyboardCallbackLock);
		m_KeyboardCallbacks.Erase(pObj);
	}

	BV_INLINE HWND GetHandle() const { return m_hWnd; }

private:
	void GetWindowDescForMode(const WindowMode windowMode, WindowDesc& windowDesc, DWORD& style, DWORD& exStyle);
	void NotifySizeChanged();
	void NotifyKeyboard(u32 vkCode, u32 scanCode, bool isKeyDown);
	void RequestCreate(const char* const pName);
	void RequestDestroy();
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hWnd = nullptr;

	BvEvent m_Event;
	BvRobinMap<void*, std::function<void(u32, u32)>> m_SizeChangedCallbacks;
	BvRobinMap<void*, std::function<void(u32, u32, bool)>> m_KeyboardCallbacks;
	BvSpinlock m_SizeChangedCallbackLock;
	BvSpinlock m_KeyboardCallbackLock;
	WindowDesc m_WindowDesc;

	u32 m_FullscreenWidth = 0;
	u32 m_FullscreenHeight = 0;

	u32 m_WindowedWidth = 0;
	u32 m_WindowedHeight = 0;
	i32 m_WindowedPosX = 0;
	i32 m_WindowedPosY = 0;

	bool m_IsActive = false;
	bool m_IsResizing = false;
	bool m_IsMinimized = false;
	bool m_IsMaximized = false;
};
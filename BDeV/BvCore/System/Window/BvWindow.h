#pragma once


#include "BvCore/BvDefines.h"


class BvWindow
{
public:
	BvWindow();
	~BvWindow();

	void Create(const u32 width, const u32 height);
	void Destroy();

	void Show();
	void Hide();

	void SetUpdated() { m_NeedsUpdate = false; }

	const u32 GetWidth() const { return m_Width; }
	const u32 GetHeight() const { return m_Height; }

	const bool IsRunning() const { return m_IsRunning; }
	const bool IsPaused() const { return m_IsPaused; }
	const bool NeedsUpdate() const { return m_NeedsUpdate; }
	const bool IsResizing() const { return m_IsResizing; }
	const bool IsMinimized() const { return m_IsMinimized; }
	const bool IsMaximized() const { return m_IsMaximized; }

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	BV_INLINE HWND GetHandle() const { return m_hWnd; }

	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	HWND m_hWnd = nullptr;
#endif

	u32 m_Width = 0;
	u32 m_Height = 0;

	bool m_IsRunning = false;
	bool m_IsPaused = false;
	bool m_NeedsUpdate = false;
	bool m_IsResizing = false;
	bool m_IsMinimized = false;
	bool m_IsMaximized = false;
};
#pragma once


#include "BDeV/Core/System/Window/BvWindowCommon.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"
#include <atomic>
#include <utility>


class BvWindow final
{
public:
	friend class BvApplication;

	explicit BvWindow(BvApplication* pApplication, const WindowDesc& windowDesc);
	~BvWindow();

	void ChangeWindow(const WindowDesc& windowDesc);
	void Reshape(i32 x, i32 y, u32 width, u32 height);
	void Resize(u32 width, u32 height);
	void Move(i32 x, i32 y);
	void SetFullscreen(bool value, BvMonitor* pMonitor = nullptr);
	void Minimize();
	void Maximize();
	void Restore();
	void Show();
	void Hide();
	void SetFocus();
	void Flash();
	void SetText(const char* pText);

	bool IsMinimized() const;
	bool IsMaximized() const;
	bool IsVisible() const;
	bool HasFocus() const;

	BV_INLINE std::pair<u32, u32> GetSize() const { return std::make_pair(m_WindowDesc.m_Width, m_WindowDesc.m_Height); }
	BV_INLINE std::pair<i32, i32> GetPosition() const { return std::make_pair(m_WindowDesc.m_X, m_WindowDesc.m_Y); }
	BV_INLINE i32 GetX() const { return m_WindowDesc.m_X; }
	BV_INLINE i32 GetY() const { return m_WindowDesc.m_Y; }
	BV_INLINE u32 GetWidth() const { return m_WindowDesc.m_Width; }
	BV_INLINE u32 GetHeight() const { return m_WindowDesc.m_Height; }
	BV_INLINE bool IsFullScreen() const { return m_WindowDesc.m_Fullscreen; }
	BV_INLINE const WindowDesc& GetWindowDesc() const { return m_WindowDesc; }

	BV_INLINE bool IsClosed() const { return m_IsClosed; }
	BV_INLINE OSWindowHandle GetHandle() const { return m_hWnd; }

private:
	void OnSizeChanged(u32 width, u32 height);
	void OnPosChanged(i32 x, i32 y);
	void OnClose();

	void Create();
	void Destroy();

private:
	BvApplication* m_pApplication = nullptr;
	OSWindowHandle m_hWnd = kNullOSWindowHandle;
	WINDOWPLACEMENT m_PreFullscreenPosition{ sizeof(WINDOWPLACEMENT) };
	WindowDesc m_WindowDesc;
	std::atomic<i32> m_RefCount;
	bool m_IsVisible = false;
	bool m_FirstTimeShow = true;
	bool m_IsClosed = false;
	bool m_Recycle = false;
};
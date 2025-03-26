#pragma once


#include "BDeV/Core/System/Window/BvWindowCommon.h"
#include "BDeV/Core/Utils/BvEvent.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"
#include <atomic>


class BvWindow final
{
public:
	friend class BvApplication;

	explicit BvWindow(BvApplication* pApplication, const WindowDesc& windowDesc);
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

	bool IsMinimized() const;
	bool IsMaximized() const;
	bool IsVisible() const;
	bool HasFocus() const;

	std::pair<u32, u32> GetSize() const { return std::make_pair(m_Width, m_Height); }
	std::pair<i32, i32> GetPosition() const { return std::make_pair(m_X, m_Y); }
	i32 GetX() const { return m_X; }
	i32 GetY() const { return m_Y; }
	u32 GetWidth() const { return m_Width; }
	u32 GetHeight() const { return m_Height; }
	WindowMode GetWindowMode() const { return m_WindowMode; }
	const WindowDesc& GetWindowDesc() const { return m_WindowDesc; }

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
	OSWindowHandle m_hWnd = nullptr;
	WINDOWPLACEMENT m_PreFullscreenPosition{};
	WindowDesc m_WindowDesc;
	u32 m_Width = 800;
	u32 m_Height = 640;
	i32 m_X = 0;
	i32 m_Y = 0;
	WindowMode m_WindowMode = WindowMode::kWindowed;
	bool m_IsVisible = false;
	bool m_FirstTimeShow = true;
	bool m_IsClosed = false;
	bool m_Recycle = false;
};
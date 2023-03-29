#pragma once


#include "BDeV\System\Window\BvWindow.h"
#include <Windows.h>


class BvWindowWindows final : public BvWindow
{
public:
	static constexpr const char* const s_WindowClassName = "BDeVWindowClass";
	static constexpr const char* const s_DefaultWindowName = "BDeV Window";

	explicit BvWindowWindows(const WindowDesc& windowDesc, WNDPROC wndProc);
	~BvWindowWindows();

	void Resize(u32 width, u32 height) override;
	void Move(i32 x, i32 y) override;
	void MoveAndResize(i32 x, i32 y, u32 width, u32 height) override;
	void Minimize() override;
	void Maximize() override;
	void Restore() override;
	void Show() override;
	void Hide() override;
	void SetFocus() override;
	void Flash() override;
	void DestroyOnClose(bool value) override;

	bool IsMinimized() const override;
	bool IsMaximized() const override;
	bool IsVisible() const override;
	bool HasFocus() const override;
	bool IsValid() const override;
	bool DestroyOnClose() const override;

	i32 GetX() const override { return m_X; }
	i32 GetY() const override { return m_Y; }
	u32 GetWidth() const override { return m_Width; }
	u32 GetHeight() const override { return m_Height; }

	LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	BV_INLINE HWND GetHandle() const { return m_hWnd; }

private:
	void Create(const WindowDesc& windowDesc, WNDPROC wndProc);
	void Destroy();

private:
	HWND m_hWnd = nullptr;
	HMODULE m_hModule = nullptr;
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
	WindowState m_CurrentState = WindowState::Restored;
};
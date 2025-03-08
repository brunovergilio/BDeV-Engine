#pragma once


#include "BDeV/Core/Container/BvVector.h"
#include <BDeV/Core/Container/BvString.h>
#include "BDeV/Core/System/BvPlatformHeaders.h"


class BvWindow;


struct VideoMode
{
	u32 m_BitsPerPixel = 0;
	u32 m_Width = 0;
	u32 m_Height = 0;
	u32 m_Frequency = 0;
};


class BvMonitor
{
	BV_NOCOPYMOVE(BvMonitor);

public:
	friend class BvMonitorHelper;

	BvMonitor(OSMonitorHandle hMonitor);
	~BvMonitor();

	struct Rect
	{
		i32 m_Left = 0;
		i32 m_Top = 0;
		i32 m_Right = 0;
		i32 m_Bottom = 0;
	};

	static const BvVector<BvMonitor*>& GetMonitors();
	static BvMonitor* FromPoint(i32 x, i32 y);
	static BvMonitor* FromWindow(const BvWindow* pWindow);
	static BvMonitor* Primary();

	BV_INLINE const BvVector<VideoMode>& GetVideoModes() const { return m_VideoModes; }
	BV_INLINE const VideoMode& GetDesktopVideoMode() const { return m_DesktopVideoMode; }
	BV_INLINE const Rect& GetFullscreenArea() const { return m_FullscreenArea; }
	BV_INLINE const Rect& GetMaximizedArea() const { return m_MaximizedArea; }
	BV_INLINE bool IsPrimary() const { return m_IsPrimary; }
	BV_INLINE const BvString& GetName() const { return m_Name; }
	BV_INLINE f32 GetDPIScaleFactor() const { return m_DPIScaleFactor; }

	BV_INLINE OSMonitorHandle GetHandle() const { return m_hMonitor; }

protected:
	OSMonitorHandle m_hMonitor = nullptr;
	Rect m_FullscreenArea;
	Rect m_MaximizedArea;
	f32 m_DPIScaleFactor = 1.0f;
	bool m_IsPrimary = false;
	BvVector<VideoMode> m_VideoModes;
	VideoMode m_DesktopVideoMode;
	BvString m_Name;
};
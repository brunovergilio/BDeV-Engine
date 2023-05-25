#pragma once


#include "BDeV/Container/BvVector.h"
#include <BDeV/Container/BvString.h>

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include <Windows.h>
#endif


struct BV_API VideoMode
{
	u32 m_BitsPerPixel = 0;
	u32 m_Width = 0;
	u32 m_Height = 0;
	u32 m_Frequency = 0;
};


class BV_API BvMonitor
{
	BV_NOCOPYMOVE(BvMonitor);

public:
	friend class BvMonitorStartUp;

	struct Rect
	{
		i32 m_Left = 0;
		i32 m_Top = 0;
		i32 m_Right = 0;
		i32 m_Bottom = 0;
	};

	BV_INLINE const BvVector<VideoMode>& GetVideoModes() const { return m_VideoModes; }
	BV_INLINE const VideoMode& GetDesktopVideoMode() const { return m_DesktopVideoMode; }
	BV_INLINE const Rect& GetFullscreenArea() const { return m_FullscreenArea; }
	BV_INLINE const Rect& GetMaximizedArea() const { return m_MaximizedArea; }
	BV_INLINE bool IsPrimary() const { return m_IsPrimary; }
	BV_INLINE const BvString& GetName() const { return m_Name; }
	BV_INLINE f32 GetDPIScaleFactor() const { return m_DPIScaleFactor; }

#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	BV_INLINE HMONITOR GetHandle() const { return m_hMonitor; }
#endif

private:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	BvMonitor(HMONITOR hMonitor);
#endif
	~BvMonitor();

protected:
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	HMONITOR m_hMonitor = nullptr;
#endif
	Rect m_FullscreenArea;
	Rect m_MaximizedArea;
	f32 m_DPIScaleFactor = 1.0f;
	bool m_IsPrimary = false;
	BvVector<VideoMode> m_VideoModes;
	VideoMode m_DesktopVideoMode;
	BvString m_Name;
};


BV_API const BvVector<BvMonitor*>& GetMonitors();
BV_API BvMonitor* GetMonitorFromPoint(i32 x, i32 y);
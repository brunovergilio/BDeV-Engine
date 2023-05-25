#include "BDeV/System/Window/BvMonitor.h"


BvMonitor::BvMonitor(HMONITOR hMonitor)
	: m_hMonitor(hMonitor)
{
	MONITORINFOEXA monitorInfo{ sizeof(MONITORINFOEXA) };
	GetMonitorInfoA(m_hMonitor, &monitorInfo);

	m_FullscreenArea.m_Left = monitorInfo.rcMonitor.left;
	m_FullscreenArea.m_Top = monitorInfo.rcMonitor.top;
	m_FullscreenArea.m_Right = monitorInfo.rcMonitor.right;
	m_FullscreenArea.m_Bottom = monitorInfo.rcMonitor.bottom;

	m_MaximizedArea.m_Left = monitorInfo.rcWork.left;
	m_MaximizedArea.m_Top = monitorInfo.rcWork.top;
	m_MaximizedArea.m_Right = monitorInfo.rcWork.right;
	m_MaximizedArea.m_Bottom = monitorInfo.rcWork.bottom;

	if (monitorInfo.dwFlags & MONITORINFOF_PRIMARY)
	{
		m_IsPrimary = true;
	}

	DEVMODEA devMode{ sizeof(DEVMODEA) };
	DWORD i = 0;
	while (EnumDisplaySettingsA(monitorInfo.szDevice, i++, &devMode))
	{
		m_VideoModes.PushBack(VideoMode{ devMode.dmBitsPerPel, devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmDisplayFrequency });
	}

	EnumDisplaySettingsA(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode);
	m_DesktopVideoMode = std::move(VideoMode{ devMode.dmBitsPerPel, devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmDisplayFrequency });

	m_DPIScaleFactor = f32(m_DesktopVideoMode.m_Width) / f32(m_FullscreenArea.m_Right - m_FullscreenArea.m_Left);

	m_Name = monitorInfo.szDevice;
}


BvMonitor::~BvMonitor()
{
}


class BvMonitorStartUp
{
	BV_NOCOPYMOVE(BvMonitorStartUp);

public:
	BvMonitorStartUp()
	{
		BvVector<HMONITOR> hMonitors;
		EnumDisplayMonitors(0, nullptr, &EnumMonitorCallback, (LPARAM)&hMonitors);

		for (auto hMonitor : hMonitors)
		{
			m_MonitorInfos.PushBack(new BvMonitor(hMonitor));
		}
	}

	~BvMonitorStartUp()
	{
		for (auto pMonitorInfo : m_MonitorInfos)
		{
			delete pMonitorInfo;
		}
		m_MonitorInfos.Clear();
	}

	static BOOL CALLBACK EnumMonitorCallback(HMONITOR hMonitor, HDC hdc, LPRECT rect, LPARAM lParam)
	{
		BvVector<HMONITOR>* hMonitors = (BvVector<HMONITOR>*)lParam;
		hMonitors->PushBack(hMonitor);

		return TRUE;
	};

public:
	BvVector<BvMonitor*> m_MonitorInfos;
};


const BvVector<BvMonitor*>& GetMonitors()
{
	static BvMonitorStartUp monitorData;
	return monitorData.m_MonitorInfos;
}


BvMonitor* GetMonitorFromPoint(i32 x, i32 y)
{
	HMONITOR hMonitor = MonitorFromPoint(POINT{ x,y }, MONITOR_DEFAULTTONULL);
	if (hMonitor)
	{
		for (auto pMonitor : GetMonitors())
		{
			auto pWinMonitor = reinterpret_cast<BvMonitor*>(pMonitor);
			if (pWinMonitor->GetHandle() == hMonitor)
			{
				return pMonitor;
			}
		}
	}

	return nullptr;
}
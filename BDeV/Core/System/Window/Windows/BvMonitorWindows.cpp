#include "BDeV/Core/System/Window/BvMonitor.h"
#include "BDeV/Core/System/Window/BvWindow.h"
#include "BDeV/Core/Utils/BvText.h"


BvMonitor::BvMonitor(OSMonitorHandle hMonitor)
	: m_hMonitor(hMonitor)
{
	MONITORINFOEXW monitorInfo{ sizeof(MONITORINFOEXW) };
	GetMonitorInfoW(m_hMonitor, &monitorInfo);

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

	DEVMODEW devMode{ sizeof(DEVMODEW) };
	DWORD i = 0;
	while (EnumDisplaySettingsW(monitorInfo.szDevice, i++, &devMode))
	{
		m_VideoModes.PushBack(VideoMode{ devMode.dmBitsPerPel, devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmDisplayFrequency });
	}

	EnumDisplaySettingsW(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode);
	m_DesktopVideoMode = std::move(VideoMode{ devMode.dmBitsPerPel, devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmDisplayFrequency });

	m_DPIScaleFactor = f32(m_DesktopVideoMode.m_Width) / f32(m_FullscreenArea.m_Right - m_FullscreenArea.m_Left);

	{
		auto sizeNeeded = sizeof(monitorInfo.szDevice);
		char* pMonitorName = (char*)BV_STACK_ALLOC(sizeNeeded);
		BvTextUtilities::ConvertWideCharToUTF8Char(monitorInfo.szDevice, 0, pMonitorName, sizeNeeded);
		m_Name = pMonitorName;
	}
}


BvMonitor::~BvMonitor()
{
}


class BvMonitorHelper
{
	BV_NOCOPYMOVE(BvMonitorHelper);

public:
	BvMonitorHelper()
	{
		BvVector<HMONITOR> monitorHandles;
		EnumDisplayMonitors(0, nullptr, &EnumMonitorCallback, (LPARAM)&monitorHandles);

		for (auto hMonitor : monitorHandles)
		{
			m_MonitorInfos.PushBack(BV_NEW(BvMonitor)(hMonitor));
		}
	}

	~BvMonitorHelper()
	{
		for (auto pMonitorInfo : m_MonitorInfos)
		{
			BV_DELETE(pMonitorInfo);
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


const BvVector<BvMonitor*>& BvMonitor::GetMonitors()
{
	static BvMonitorHelper monitorData;
	return monitorData.m_MonitorInfos;
}


BvMonitor* BvMonitor::FromPoint(i32 x, i32 y)
{
	HMONITOR hMonitor = MonitorFromPoint(POINT{ x,y }, MONITOR_DEFAULTTONEAREST);
	if (hMonitor)
	{
		for (auto pMonitor : GetMonitors())
		{
			if (pMonitor->GetHandle() == hMonitor)
			{
				return pMonitor;
			}
		}
	}

	return nullptr;
}


BvMonitor* BvMonitor::FromWindow(const BvWindow* pWindow)
{
	HMONITOR hMonitor = MonitorFromWindow((HWND)pWindow->GetHandle(), MONITOR_DEFAULTTONEAREST);
	if (hMonitor)
	{
		for (auto pMonitor : GetMonitors())
		{
			if (pMonitor->GetHandle() == hMonitor)
			{
				return pMonitor;
			}
		}
	}

	return nullptr;
}


BvMonitor* BvMonitor::Primary()
{
	for (auto pMonitor : GetMonitors())
	{
		if (pMonitor->IsPrimary())
		{
			return pMonitor;
		}
	}

	return nullptr;
}
#pragma once


#include "BDeV/System/Window/BvMonitor.h"
#include <Windows.h>


class BvMonitorWindows : public BvMonitor
{
	BV_NOCOPYMOVE(BvMonitorWindows);

public:
	BvMonitorWindows(HMONITOR hMonitor);
	~BvMonitorWindows();

	BV_INLINE HMONITOR GetHandle() const { return m_hMonitor; }

private:
	HMONITOR m_hMonitor = nullptr;
};
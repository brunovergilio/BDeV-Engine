#include "BvMemoryTracker.h"


// ===============================================
// Simple Memory Tracker
// ===============================================
BvSimpleMemoryTracker::BvSimpleMemoryTracker()
{
}


BvSimpleMemoryTracker::~BvSimpleMemoryTracker()
{
}


void BvSimpleMemoryTracker::OnAllocation(void* pMem, size_t size, size_t alignment, const BvSourceInfo& sourceInfo)
{
	m_NumAllocations++;
}


void BvSimpleMemoryTracker::OnDeallocation(void* pMem)
{
	m_NumAllocations--;
}


// ===============================================
// Extended Memory Tracker
// ===============================================
BvExtendedMemoryTracker::BvExtendedMemoryTracker()
{
}


BvExtendedMemoryTracker::~BvExtendedMemoryTracker()
{
}


void BvExtendedMemoryTracker::OnAllocation(void* pMem, size_t size, size_t alignment, const BvSourceInfo& sourceInfo)
{
	auto& trackingData = m_Allocations[pMem];
	trackingData.m_SourceInfo = sourceInfo;
	trackingData.m_Size = size;
	trackingData.m_Alignment = alignment;
}


void BvExtendedMemoryTracker::OnDeallocation(void* pMem)
{
	m_Allocations.Erase(pMem);
}


void BvExtendedMemoryTracker::GetTrackingInfo(void* pMem, TrackingInfo& trackingInfo) const
{
	auto iter = m_Allocations.FindKey(pMem);
	if (iter != m_Allocations.cend())
	{
		trackingInfo.m_SourceInfo = iter->second.m_SourceInfo;
		trackingInfo.m_Size = iter->second.m_Size;
		trackingInfo.m_Alignment = iter->second.m_Alignment;
	}
}


// ===============================================
// Full Memory Tracker
// ===============================================
BvFullMemoryTracker::BvFullMemoryTracker()
{
}


BvFullMemoryTracker::~BvFullMemoryTracker()
{
}


void BvFullMemoryTracker::OnAllocation(void* pMem, size_t size, size_t alignment, const BvSourceInfo& sourceInfo)
{
	auto& trackingData = m_Allocations[pMem];
	trackingData.m_SourceInfo = sourceInfo;
	trackingData.m_Size = size;
	trackingData.m_Alignment = alignment;

	BvProcess::GetStackTrace(trackingData.m_StackTrace, 3);
}


void BvFullMemoryTracker::OnDeallocation(void* pMem)
{
	m_Allocations.Erase(pMem);
}


void BvFullMemoryTracker::GetTrackingInfo(void* pMem, TrackingInfo& trackingInfo) const
{
	auto iter = m_Allocations.FindKey(pMem);
	if (iter != m_Allocations.cend())
	{
		trackingInfo.m_SourceInfo = iter->second.m_SourceInfo;

		trackingInfo.m_Size = iter->second.m_Size;
		trackingInfo.m_Alignment = iter->second.m_Alignment;

		trackingInfo.m_pStackTrace = &iter->second.m_StackTrace;
	}
}
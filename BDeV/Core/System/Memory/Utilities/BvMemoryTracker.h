#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Threading/BvProcess.h"
#include "BDeV/Core/Container/BvRobinMap.h"


struct TrackingInfo
{
	size_t m_Size;
	size_t m_Alignment;
	BvSourceInfo m_SourceInfo;
	const BvStackTrace* m_pStackTrace;
};


class BvNoMemoryTracker
{
	BV_NOCOPYMOVE(BvNoMemoryTracker);
public:
	BvNoMemoryTracker() {}
	~BvNoMemoryTracker() {}

	BV_INLINE void OnAllocation(void* pMem, size_t size, size_t alignment, const BvSourceInfo& sourceInfo) {}
	BV_INLINE void OnDeallocation(void* pMem) {}

	BV_INLINE u32 GetNumAllocations() const { return 0; }
	BV_INLINE void GetTrackingInfo(void* pMem, TrackingInfo& trackingInfo) const {}
};


class BvSimpleMemoryTracker
{
	BV_NOCOPYMOVE(BvSimpleMemoryTracker);
public:
	BvSimpleMemoryTracker();
	~BvSimpleMemoryTracker();

	void OnAllocation(void* pMem, size_t size, size_t alignment, const BvSourceInfo& sourceInfo);
	void OnDeallocation(void* pMem);

	BV_INLINE u32 GetNumAllocations() const { return m_NumAllocations; }
	BV_INLINE void GetTrackingInfo(void* pMem, TrackingInfo& trackingInfo) const {}

private:
	u32 m_NumAllocations = 0;
};


class BvExtendedMemoryTracker
{
	BV_NOCOPYMOVE(BvExtendedMemoryTracker);
public:
	BvExtendedMemoryTracker();
	~BvExtendedMemoryTracker();

	void OnAllocation(void* pMem, size_t size, size_t alignment, const BvSourceInfo& sourceInfo);
	void OnDeallocation(void* pMem);

	BV_INLINE u32 GetNumAllocations() const { return m_Allocations.Size(); }
	void GetTrackingInfo(void* pMem, TrackingInfo& trackingInfo) const;

private:
	struct TrackingData
	{
		BvSourceInfo m_SourceInfo;
		size_t m_Size;
		size_t m_Alignment;
	};
	BvRobinMap<void*, TrackingData> m_Allocations;
};


class BvFullMemoryTracker
{
	BV_NOCOPYMOVE(BvFullMemoryTracker);
public:
	BvFullMemoryTracker();
	~BvFullMemoryTracker();

	void OnAllocation(void* pMem, size_t size, size_t alignment, const BvSourceInfo& sourceInfo);
	void OnDeallocation(void* pMem);

	BV_INLINE u32 GetNumAllocations() const { return m_Allocations.Size(); }
	void GetTrackingInfo(void* pMem, TrackingInfo& trackingInfo) const;

private:
	struct TrackingData
	{
		BvStackTrace m_StackTrace;
		BvSourceInfo m_SourceInfo;
		size_t m_Size;
		size_t m_Alignment;
	};
	BvRobinMap<void*, TrackingData> m_Allocations;
};
#pragma once


#include "BvRender/BvFence.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvRender/BvCommon.h"


constexpr u32 kMaxCommandBuffersPerSubmission = 16;
constexpr u32 kMaxWaitSignalSemaphoresSubmission = 8;


class BvCommandBuffer;
class BvSemaphore;


struct SubmitInfo
{
	BvCommandBuffer ** ppCommandBuffers = nullptr;
	BvSemaphore ** ppWaitSemaphores = nullptr;
	u64 * pWaitValues = nullptr;
	BvSemaphore ** ppSignalSemaphores = nullptr;
	u64 * pSignalValues = nullptr;
	u32 commandBufferCount = 0;
	u32 waitSemaphoreCount = 0;
	u32 signalSemaphoreCount = 0;
};


class BvCommandQueue
{
	BV_NOCOPYMOVE(BvCommandQueue);

public:
	virtual void Submit(const SubmitInfo & submitInfo) = 0;
	virtual void WaitIdle() = 0;
	BV_INLINE const QueueFamilyType GetFamilyType() const { return m_QueueFamilyType; }

protected:
	BvCommandQueue(const QueueFamilyType queueFamilyType)
		: m_QueueFamilyType(queueFamilyType) {}
	virtual ~BvCommandQueue() = 0 {}

protected:
	QueueFamilyType m_QueueFamilyType;
};
#include "BvCommandQueueVk.h"
#include "BvRenderDeviceVk.h"
#include "BvCommandBufferVk.h"
#include "BvFenceVk.h"
#include "BvSemaphoreVk.h"
#include "BvSwapChainVk.h"


BvCommandQueueVk::BvCommandQueueVk(const BvRenderDeviceVk & device, const QueueFamilyType queueFamilyType, const u32 queueIndex)
	: BvCommandQueue(queueFamilyType), m_Device(device), m_QueueIndex(queueIndex)
{
	switch (queueFamilyType)
	{
	case QueueFamilyType::kGraphics:
		m_QueueFamilyIndex = device.GetGPUInfo().m_GraphicsQueueIndex;
		break;
	case QueueFamilyType::kCompute:
		m_QueueFamilyIndex = device.GetGPUInfo().m_ComputeQueueIndex;
		break;
	case QueueFamilyType::kTransfer:
		m_QueueFamilyIndex = device.GetGPUInfo().m_TransferQueueIndex;
		break;
	}

	m_Device.GetDeviceFunctions().vkGetDeviceQueue(device.GetHandle(), m_QueueFamilyIndex, queueIndex, &m_Queue);
}


BvCommandQueueVk::~BvCommandQueueVk()
{
}


void BvCommandQueueVk::Submit(const SubmitInfo & submitInfo)
{
	BvAssert(submitInfo.commandBufferCount <= kMaxCommandBuffersPerSubmission, "Command buffer count greater than limit");
	BvAssert(submitInfo.waitSemaphoreCount <= kMaxWaitSignalSemaphoresSubmission, "Wait semaphore count greater than limit");
	BvAssert(submitInfo.signalSemaphoreCount <= kMaxWaitSignalSemaphoresSubmission, "Signal semaphore count greater than limit");
	
	BvFixedVector<u64, kMaxWaitSignalSemaphoresSubmission> waitValues(submitInfo.waitSemaphoreCount + m_SwapChains.Size());
	for (auto i = 0u; i < submitInfo.waitSemaphoreCount; i++)
	{
		waitValues[i] = submitInfo.pWaitValues ? submitInfo.pWaitValues[i] : 0;
	}

	BvFixedVector<u64, kMaxWaitSignalSemaphoresSubmission> signalValues(submitInfo.signalSemaphoreCount);
	for (auto i = 0u; i < submitInfo.signalSemaphoreCount; i++)
	{
		signalValues[i] = submitInfo.pSignalValues ? submitInfo.pSignalValues[i] : 0;
	}

	BvFixedVector<VkSemaphore, kMaxWaitSignalSemaphoresSubmission> waitSemaphores(submitInfo.waitSemaphoreCount, {});
	for (auto i = 0u; i < submitInfo.waitSemaphoreCount; i++)
	{
		auto smVk = static_cast<BvSemaphoreVk *>(submitInfo.ppWaitSemaphores[i]);
		waitSemaphores[i] = smVk->GetHandle();
	}
	for (auto & pSwapChain : m_SwapChains)
	{
		waitSemaphores.PushBack(pSwapChain->GetCurrentSemaphore()->GetHandle());
	}

	BvFixedVector<VkSemaphore, kMaxWaitSignalSemaphoresSubmission> signalSemaphores(submitInfo.signalSemaphoreCount, {});
	for (auto i = 0u; i < submitInfo.signalSemaphoreCount; i++)
	{
		auto smVk = static_cast<BvSemaphoreVk *>(submitInfo.ppSignalSemaphores[i]);
		signalSemaphores[i] = smVk->GetHandle();
	}

	VkPipelineStageFlags waitStageFlags = 0;

	BvFixedVector<VkCommandBuffer, kMaxCommandBuffersPerSubmission> commandBuffers(submitInfo.commandBufferCount, {});
	for (auto i = 0u; i < commandBuffers.Size(); i++)
	{
		auto cbVk = static_cast<BvCommandBufferVk *>(submitInfo.ppCommandBuffers[i]);
		commandBuffers[i] = cbVk->GetHandle();

		waitStageFlags |= cbVk->GetWaitStageFlags();

		for (auto & semaphore : cbVk->GetSwapChainSignalSemaphores())
		{
			signalSemaphores.PushBack(semaphore);
			signalValues.PushBack(0);
		}
	}

	BvFixedVector<VkPipelineStageFlags, kMaxWaitSignalSemaphoresSubmission> waitStages(waitSemaphores.Size(), {});
	for (auto i = 0u; i < waitStages.Size(); i++)
	{
		waitStages[i] = waitStageFlags;
	}

	VkTimelineSemaphoreSubmitInfo timelineInfo{};
	timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	timelineInfo.waitSemaphoreValueCount = (u32)waitValues.Size();
	timelineInfo.pWaitSemaphoreValues = waitValues.Data();
	timelineInfo.signalSemaphoreValueCount = (u32)signalValues.Size();
	timelineInfo.pSignalSemaphoreValues = signalValues.Data();

	VkSubmitInfo submitInfoVK{};
	submitInfoVK.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfoVK.pNext = &timelineInfo;
	
	if (submitInfo.commandBufferCount > 0)
	{
		submitInfoVK.commandBufferCount = submitInfo.commandBufferCount;
		submitInfoVK.pCommandBuffers = commandBuffers.Data();
	}
	if (waitSemaphores.Size() > 0)
	{
		submitInfoVK.waitSemaphoreCount = (u32)waitSemaphores.Size();
		submitInfoVK.pWaitSemaphores = waitSemaphores.Data();
		submitInfoVK.pWaitDstStageMask = waitStages.Data();
	}
	if (signalSemaphores.Size() > 0)
	{
		submitInfoVK.signalSemaphoreCount = (u32)signalSemaphores.Size();
		submitInfoVK.pSignalSemaphores = signalSemaphores.Data();
	}

	auto result = m_Device.GetDeviceFunctions().vkQueueSubmit(m_Queue, 1, &submitInfoVK, VK_NULL_HANDLE);

	m_SwapChains.Clear();
}


void BvCommandQueueVk::WaitIdle()
{
	auto result = m_Device.GetDeviceFunctions().vkQueueWaitIdle(m_Queue);
}


void BvCommandQueueVk::AddSwapChain(BvSwapChainVk * pSwapChain)
{
	if (std::find(m_SwapChains.begin(), m_SwapChains.end(), pSwapChain) == m_SwapChains.end())
	{
		m_SwapChains.EmplaceBack(pSwapChain);
	}
}
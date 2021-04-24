#include "BvCommandQueueVk.h"
#include "BvRenderDeviceVk.h"
#include "BvCommandBufferVk.h"
#include "BvFenceVk.h"
#include "BvSemaphoreVk.h"
#include "BvSwapChainVk.h"


BvCommandQueueVk::BvCommandQueueVk(const BvRenderDeviceVk & device, const QueueFamilyType queueFamilyType, const u32 queueIndex)
	: BvCommandQueue(queueFamilyType), m_Device(device), m_QueueIndex(queueIndex), m_pSubmitInfo(new SubmitInfoData())
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
	delete m_pSubmitInfo;
}


void BvCommandQueueVk::Submit(const SubmitInfo& submitInfo)
{
	// Add new timeline object
	m_pSubmitInfo->m_TimelineSemaphoreInfos.PushBack({});
	auto& timelineSI = m_pSubmitInfo->m_TimelineSemaphoreInfos.Back();
	timelineSI.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	
	timelineSI.waitSemaphoreValueCount = (u32)submitInfo.waitSemaphoreCount;
	for (auto i = 0u; i < submitInfo.waitSemaphoreCount; i++)
	{
		m_pSubmitInfo->m_WaitSemaphores.EmplaceBack(static_cast<BvSemaphoreVk*>(submitInfo.ppWaitSemaphores[i])->GetHandle());
		m_pSubmitInfo->m_WaitSemaphoreValues.EmplaceBack(submitInfo.pWaitValues[i]);
	}
	
	timelineSI.signalSemaphoreValueCount = (u32)submitInfo.signalSemaphoreCount;
	for (auto i = 0u; i < submitInfo.signalSemaphoreCount; i++)
	{
		m_pSubmitInfo->m_SignalSemaphores.PushBack(static_cast<BvSemaphoreVk*>(submitInfo.ppSignalSemaphores[i])->GetHandle());
		m_pSubmitInfo->m_SignalSemaphoreValues.EmplaceBack(submitInfo.pSignalValues[i]);
	}

	VkPipelineStageFlags waitStageFlags = 0;

	for (auto i = 0u; i < submitInfo.commandBufferCount; i++)
	{
		auto cbVk = static_cast<BvCommandBufferVk*>(submitInfo.ppCommandBuffers[i]);
		m_pSubmitInfo->m_CommandBuffers.EmplaceBack(cbVk->GetHandle());

		waitStageFlags |= cbVk->GetWaitStageFlags();

		// Any swap chains affected will have a wait semaphore and a signal semaphore (both binary)
		auto& swapChains = cbVk->GetSwapChains();
		for (auto j = 0u; j < swapChains.Size(); j++)
		{
			timelineSI.waitSemaphoreValueCount++;
			m_pSubmitInfo->m_WaitSemaphores.EmplaceBack(swapChains[j]->GetCurrentImageAcquiredSemaphore()->GetHandle());
			m_pSubmitInfo->m_WaitSemaphoreValues.EmplaceBack(0);

			timelineSI.signalSemaphoreValueCount++;
			m_pSubmitInfo->m_SignalSemaphores.EmplaceBack(swapChains[j]->GetCurrentRenderCompleteSemaphore()->GetHandle());
			m_pSubmitInfo->m_SignalSemaphoreValues.EmplaceBack(0);
		}
	}

	for (auto i = 0u; i < timelineSI.waitSemaphoreValueCount; i++)
	{
		m_pSubmitInfo->m_WaitStageFlags.EmplaceBack(waitStageFlags);
	}

	m_pSubmitInfo->m_SubmitInfos.PushBack({});
	auto& si = m_pSubmitInfo->m_SubmitInfos.Back();
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.commandBufferCount = submitInfo.commandBufferCount;
	si.waitSemaphoreCount = timelineSI.waitSemaphoreValueCount;
	si.signalSemaphoreCount = timelineSI.signalSemaphoreValueCount;
}


void BvCommandQueueVk::Execute()
{
	u32 commandBufferIndex = 0;
	u32 waitSemaphoreIndex = 0;
	u32 signalSemaphoreIndex = 0;
	for (auto i = 0u; i < m_pSubmitInfo->m_SubmitInfos.Size(); i++)
	{
		if (m_pSubmitInfo->m_TimelineSemaphoreInfos[i].waitSemaphoreValueCount > 0)
		{
			m_pSubmitInfo->m_TimelineSemaphoreInfos[i].pWaitSemaphoreValues = m_pSubmitInfo->m_WaitSemaphoreValues.Data() + waitSemaphoreIndex;
		}

		if (m_pSubmitInfo->m_TimelineSemaphoreInfos[i].signalSemaphoreValueCount > 0)
		{
			m_pSubmitInfo->m_TimelineSemaphoreInfos[i].pSignalSemaphoreValues = m_pSubmitInfo->m_SignalSemaphoreValues.Data() + signalSemaphoreIndex;
		}

		m_pSubmitInfo->m_SubmitInfos[i].pNext = &m_pSubmitInfo->m_TimelineSemaphoreInfos[i];
		if (m_pSubmitInfo->m_SubmitInfos[i].commandBufferCount > 0)
		{
			m_pSubmitInfo->m_SubmitInfos[i].pCommandBuffers = m_pSubmitInfo->m_CommandBuffers.Data() + commandBufferIndex;
		}

		if (m_pSubmitInfo->m_SubmitInfos[i].waitSemaphoreCount > 0)
		{
			m_pSubmitInfo->m_SubmitInfos[i].pWaitSemaphores = m_pSubmitInfo->m_WaitSemaphores.Data() + waitSemaphoreIndex;
			m_pSubmitInfo->m_SubmitInfos[i].pWaitDstStageMask = m_pSubmitInfo->m_WaitStageFlags.Data() + waitSemaphoreIndex;
		}

		if (m_pSubmitInfo->m_SubmitInfos[i].signalSemaphoreCount > 0)
		{
			m_pSubmitInfo->m_SubmitInfos[i].pSignalSemaphores = m_pSubmitInfo->m_SignalSemaphores.Data() + signalSemaphoreIndex;
		}

		commandBufferIndex += m_pSubmitInfo->m_SubmitInfos[i].commandBufferCount;
		waitSemaphoreIndex += m_pSubmitInfo->m_SubmitInfos[i].waitSemaphoreCount;
		signalSemaphoreIndex += m_pSubmitInfo->m_SubmitInfos[i].signalSemaphoreCount;
	}

	auto result = m_Device.GetDeviceFunctions().vkQueueSubmit(m_Queue,
		(u32)m_pSubmitInfo->m_SubmitInfos.Size(), m_pSubmitInfo->m_SubmitInfos.Data(), VK_NULL_HANDLE);
	
	m_pSubmitInfo->m_SubmitInfos.Clear();
	m_pSubmitInfo->m_TimelineSemaphoreInfos.Clear();
	m_pSubmitInfo->m_CommandBuffers.Clear();
	m_pSubmitInfo->m_WaitSemaphores.Clear();
	m_pSubmitInfo->m_WaitSemaphoreValues.Clear();
	m_pSubmitInfo->m_WaitStageFlags.Clear();
	m_pSubmitInfo->m_SignalSemaphores.Clear();
	m_pSubmitInfo->m_SignalSemaphoreValues.Clear();
}


void BvCommandQueueVk::WaitIdle()
{
	auto result = m_Device.GetDeviceFunctions().vkQueueWaitIdle(m_Queue);
}
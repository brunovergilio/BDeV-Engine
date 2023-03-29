#include "BvCommandQueueVk.h"
#include "BvRenderDeviceVk.h"
#include "BvCommandBufferVk.h"
#include "BvSemaphoreVk.h"
#include "BvSwapChainVk.h"
#include "BvFenceVk.h"
#include "BDeV/System/Debug/BvDebug.h"


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

	vkGetDeviceQueue(device.GetHandle(), m_QueueFamilyIndex, queueIndex, &m_Queue);
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

	bool timelineSemaphoreSupported = m_Device.GetGPUInfo().m_FeaturesSupported.timelineSemaphore;

	timelineSI.waitSemaphoreValueCount = (u32)submitInfo.waitSemaphoreCount;
	for (auto i = 0u; i < submitInfo.waitSemaphoreCount; i++)
	{
		auto pSemVk = static_cast<BvSemaphoreVk*>(submitInfo.ppWaitSemaphores[i]);
		m_pSubmitInfo->m_WaitSemaphores.EmplaceBack(pSemVk->GetHandle());
		m_pSubmitInfo->m_WaitSemaphoreValues.EmplaceBack(submitInfo.pWaitValues[i]);
		if (!timelineSemaphoreSupported)
		{
			auto activeSemIndex = m_ActiveSemaphores.Find(pSemVk->GetHandle());
			if (activeSemIndex != kU64Max)
			{
				m_ActiveSemaphores.Erase(activeSemIndex);
			}
		}
	}

	BvFenceVk* pFence = nullptr;
	if (!timelineSemaphoreSupported && submitInfo.signalSemaphoreCount > 0)
	{
		pFence = GetFenceManager()->GetFence(m_Device);
		pFence->IncrementUsageCount((i32)submitInfo.signalSemaphoreCount);
		m_pSubmitInfo->m_pFence = pFence;
	}
	timelineSI.signalSemaphoreValueCount = (u32)submitInfo.signalSemaphoreCount;
	for (auto i = 0u; i < submitInfo.signalSemaphoreCount; i++)
	{
		auto pSemVk = static_cast<BvSemaphoreVk*>(submitInfo.ppSignalSemaphores[i]);
		if (!timelineSemaphoreSupported)
		{
			pSemVk->AddSyncPoint(pFence, submitInfo.pSignalValues[i], m_QueueFamilyIndex, m_QueueIndex);
			auto activeSemIndex = m_ActiveSemaphores.Find(pSemVk->GetHandle());
			if (activeSemIndex != kU64Max)
			{
				m_pSubmitInfo->m_WaitSemaphores.EmplaceBack(m_ActiveSemaphores[activeSemIndex]);
				m_pSubmitInfo->m_WaitSemaphoreValues.EmplaceBack(0);
				timelineSI.waitSemaphoreValueCount++;
			}
			else
			{
				m_ActiveSemaphores.EmplaceBack(pSemVk->GetHandle());
			}
		}
		m_pSubmitInfo->m_SignalSemaphores.PushBack(pSemVk->GetHandle());
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
	bool timelineSemaphoreSupported = m_Device.GetGPUInfo().m_FeaturesSupported.timelineSemaphore;
	for (auto i = 0u; i < m_pSubmitInfo->m_SubmitInfos.Size(); i++)
	{
		if (timelineSemaphoreSupported)
		{
			if (m_pSubmitInfo->m_TimelineSemaphoreInfos[i].waitSemaphoreValueCount > 0)
			{
				m_pSubmitInfo->m_TimelineSemaphoreInfos[i].pWaitSemaphoreValues = m_pSubmitInfo->m_WaitSemaphoreValues.Data() + waitSemaphoreIndex;
			}

			if (m_pSubmitInfo->m_TimelineSemaphoreInfos[i].signalSemaphoreValueCount > 0)
			{
				m_pSubmitInfo->m_TimelineSemaphoreInfos[i].pSignalSemaphoreValues = m_pSubmitInfo->m_SignalSemaphoreValues.Data() + signalSemaphoreIndex;
			}
		}
		m_pSubmitInfo->m_SubmitInfos[i].pNext = timelineSemaphoreSupported ? &m_pSubmitInfo->m_TimelineSemaphoreInfos[i] : nullptr;

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

	auto result = vkQueueSubmit(m_Queue,
		(u32)m_pSubmitInfo->m_SubmitInfos.Size(), m_pSubmitInfo->m_SubmitInfos.Data(), m_pSubmitInfo->m_pFence ? m_pSubmitInfo->m_pFence->GetHandle() : nullptr);

	m_pSubmitInfo->m_SubmitInfos.Clear();
	m_pSubmitInfo->m_TimelineSemaphoreInfos.Clear();
	m_pSubmitInfo->m_CommandBuffers.Clear();
	m_pSubmitInfo->m_WaitSemaphores.Clear();
	m_pSubmitInfo->m_WaitSemaphoreValues.Clear();
	m_pSubmitInfo->m_WaitStageFlags.Clear();
	m_pSubmitInfo->m_SignalSemaphores.Clear();
	m_pSubmitInfo->m_SignalSemaphoreValues.Clear();
	m_pSubmitInfo->m_pFence = nullptr;
}


void BvCommandQueueVk::WaitIdle()
{
	auto result = vkQueueWaitIdle(m_Queue);
}
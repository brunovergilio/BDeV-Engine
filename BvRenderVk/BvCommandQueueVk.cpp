#include "BvCommandQueueVk.h"
#include "BvRenderDeviceVk.h"
#include "BvCommandBufferVk.h"
#include "BvSwapChainVk.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"


BvCommandQueueVk::BvCommandQueueVk()
{
}


BvCommandQueueVk::BvCommandQueueVk(VkDevice device, u32 queueFamilyIndex, u32 queueIndex)
	: m_Device(device)
{
	vkGetDeviceQueue(m_Device, queueFamilyIndex, queueIndex, &m_Queue);
}


BvCommandQueueVk::BvCommandQueueVk(BvCommandQueueVk&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvCommandQueueVk& BvCommandQueueVk::operator=(BvCommandQueueVk&& rhs) noexcept
{
	m_Device = rhs.m_Device;
	m_Queue = rhs.m_Queue;

	return *this;
}


BvCommandQueueVk::~BvCommandQueueVk()
{
}


void BvCommandQueueVk::AddWaitSemaphore(VkSemaphore waitSemaphore, u64 value)
{
	auto& waitInfo = m_WaitSemaphores.EmplaceBack();
	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	waitInfo.semaphore = waitSemaphore;
	waitInfo.value = value;
	waitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
}


void BvCommandQueueVk::AddSignalSemaphore(VkSemaphore signalSemaphore, u64 value)
{
	auto& signalInfo = m_SignalSemaphores.EmplaceBack();
	signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	signalInfo.semaphore = signalSemaphore;
	signalInfo.value = value;
	signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
}


void BvCommandQueueVk::Submit(const BvVector<BvCommandBufferVk*>& commandBuffers, VkSemaphore signalSemaphore, u64 signalValue)
{
	for (auto i = 0; i < commandBuffers.Size(); ++i)
	{
		auto& cbInfo = m_CommandBuffers.EmplaceBack();
		cbInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cbInfo.commandBuffer = commandBuffers[i]->GetHandle();
	}

	auto& signalInfo = m_SignalSemaphores.EmplaceBack();
	signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	signalInfo.semaphore = signalSemaphore;
	signalInfo.value = signalValue;
	signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

	for (auto pCommandBuffer : commandBuffers)
	{
		const auto& swapChains = pCommandBuffer->GetSwapChains();
		for (auto pSwapChain : swapChains)
		{
			if (!pSwapChain->IsReady())
			{
				continue;
			}
			auto& scSignalInfo = m_SignalSemaphores.EmplaceBack();
			scSignalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			scSignalInfo.semaphore = pSwapChain->GetCurrentRenderCompleteSemaphore();
			scSignalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

			auto& scWaitInfo = m_WaitSemaphores.EmplaceBack();
			scWaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			scWaitInfo.semaphore = pSwapChain->GetCurrentImageAcquiredSemaphore();
			scWaitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
	}

	VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
	//submitInfo.pNext = nullptr;
	//submitInfo.flags = 0;
	submitInfo.commandBufferInfoCount = (u32)m_CommandBuffers.Size();
	submitInfo.pCommandBufferInfos = m_CommandBuffers.Data();
	submitInfo.waitSemaphoreInfoCount = (u32)m_WaitSemaphores.Size();
	submitInfo.pWaitSemaphoreInfos = m_WaitSemaphores.Data();
	submitInfo.signalSemaphoreInfoCount = (u32)m_SignalSemaphores.Size();
	submitInfo.pSignalSemaphoreInfos = m_SignalSemaphores.Data();

	auto result = vkQueueSubmit2(m_Queue, 1, &submitInfo, VK_NULL_HANDLE);
	
	m_CommandBuffers.Clear();
	m_WaitSemaphores.Clear();
	m_SignalSemaphores.Clear();
}


void BvCommandQueueVk::WaitIdle()
{
	auto result = vkQueueWaitIdle(m_Queue);
}
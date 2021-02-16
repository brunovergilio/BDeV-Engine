#include "BvFenceVk.h"
#include "BvRenderDeviceVk.h"


BvFenceVk::BvFenceVk(const BvRenderDeviceVk & device, const State initialState)
	: m_Device(device), m_InitialState(initialState)
{
}


BvFenceVk::~BvFenceVk()
{
	Destroy();
}


void BvFenceVk::Create()
{
	VkFenceCreateInfo fenceCI{};
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.flags = m_InitialState == State::kSignaled ? VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT : 0;

	auto result = m_Device.GetDeviceFunctions().vkCreateFence(m_Device.GetHandle(), &fenceCI, nullptr, &m_Fence);
}


void BvFenceVk::Destroy()
{
	if (m_Fence)
	{
		m_Device.GetDeviceFunctions().vkDestroyFence(m_Device.GetHandle(), m_Fence, nullptr);
		m_Fence = VK_NULL_HANDLE;
	}
}


void BvFenceVk::Reset()
{
	auto result = m_Device.GetDeviceFunctions().vkResetFences(m_Device.GetHandle(), 1, &m_Fence);
}


bool BvFenceVk::Wait(const u64 timeout)
{
	auto result = m_Device.GetDeviceFunctions().vkWaitForFences(m_Device.GetHandle(), 1, &m_Fence, VK_TRUE, timeout);
	BvAssert(result == VK_SUCCESS || result == VK_TIMEOUT);

	return result == VK_SUCCESS;
}
#include "BvCommandQueueGl.h"


BvCommandQueueGl::BvCommandQueueGl(const BvRenderDeviceGl& device, const CommandType queueFamilyType, const u32 queueIndex)
	: BvCommandQueue(queueFamilyType), m_Device(device)
{
}


BvCommandQueueGl::~BvCommandQueueGl()
{
}


void BvCommandQueueGl::Submit(const SubmitInfo& submitInfo)
{
}


void BvCommandQueueGl::Execute()
{
}


void BvCommandQueueGl::WaitIdle()
{
}
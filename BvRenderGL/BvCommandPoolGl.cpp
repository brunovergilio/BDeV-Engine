#include "BvCommandPoolGl.h"


BvCommandPoolGl::BvCommandPoolGl(const BvRenderDeviceGl& device, const CommandPoolDesc& commandPoolDesc)
	: BvCommandPool(commandPoolDesc), m_Device(device)
{
}

BvCommandPoolGl::~BvCommandPoolGl()
{
}


void BvCommandPoolGl::AllocateCommandBuffers(u32 commandBufferCount, BvCommandBuffer** ppCommandBuffers)
{
}


void BvCommandPoolGl::FreeCommandBuffers(u32 commandBufferCount, BvCommandBuffer** ppCommandBuffers)
{
}


void BvCommandPoolGl::Reset()
{
}
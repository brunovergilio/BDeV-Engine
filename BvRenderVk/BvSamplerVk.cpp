#include "BvSamplerVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvUtilsVk.h"


BvSamplerVk::BvSamplerVk(BvRenderDeviceVk* pDevice, const SamplerDesc & samplerDesc, VkSampler sampler)
	: m_SamplerDesc(samplerDesc), m_pDevice(pDevice), m_Sampler(sampler)
{
}


BvSamplerVk::~BvSamplerVk()
{
	Destroy();
}



void BvSamplerVk::Destroy()
{
	if (m_Sampler)
	{
		VkHelpers::DestroyDeviceObject(m_pDevice->GetHandle(), m_Sampler);
		m_pDevice->OnVkHandleDestroyed(u64(m_Sampler), false);
		m_Sampler = VK_NULL_HANDLE;
	}
}
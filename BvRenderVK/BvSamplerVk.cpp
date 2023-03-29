#include "BvSamplerVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvUtilsVk.h"


BvSamplerVk::BvSamplerVk(const BvRenderDeviceVk & device, const SamplerDesc & samplerDesc)
	: BvSampler(samplerDesc), m_Device(device)
{
	Create();
}


BvSamplerVk::~BvSamplerVk()
{
	Destroy();
}


void BvSamplerVk::Create()
{
	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	//samplerCreateInfo.pNext = nullptr;
	//samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = GetVkFilter(m_SamplerDesc.m_MagFilter);
	samplerCreateInfo.minFilter = GetVkFilter(m_SamplerDesc.m_MinFilter);
	samplerCreateInfo.mipmapMode = GetVkSamplerMipmapMode(m_SamplerDesc.m_MipmapMode);
	samplerCreateInfo.addressModeU = GetVkSamplerAddressMode(m_SamplerDesc.m_AddressModeU);
	samplerCreateInfo.addressModeV = GetVkSamplerAddressMode(m_SamplerDesc.m_AddressModeV);
	samplerCreateInfo.addressModeW = GetVkSamplerAddressMode(m_SamplerDesc.m_AddressModeW);
	samplerCreateInfo.mipLodBias = m_SamplerDesc.m_MipLodBias;
	samplerCreateInfo.anisotropyEnable = m_SamplerDesc.m_AnisotropyEnable;
	samplerCreateInfo.maxAnisotropy = m_SamplerDesc.m_MaxAnisotropy;
	samplerCreateInfo.compareEnable = VkBool32(m_SamplerDesc.m_CompareEnable);
	samplerCreateInfo.compareOp = GetVkCompareOp(m_SamplerDesc.m_CompareOp);
	samplerCreateInfo.minLod = m_SamplerDesc.m_MinLod;
	samplerCreateInfo.maxLod = m_SamplerDesc.m_MaxLod;
	//samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	auto result = vkCreateSampler(m_Device.GetHandle(), &samplerCreateInfo, nullptr, &m_Sampler);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
	}
}


void BvSamplerVk::Destroy()
{
	if (m_Sampler)
	{
		vkDestroySampler(m_Device.GetHandle(), m_Sampler, nullptr);
		m_Sampler = VK_NULL_HANDLE;
	}
}
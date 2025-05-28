#include "BvSamplerVk.h"
#include "BvRenderDeviceVk.h"
#include "BvTypeConversionsVk.h"
#include "BvUtilsVk.h"


BV_VK_DEVICE_RES_DEF(BvSamplerVk)


BvSamplerVk::BvSamplerVk(BvRenderDeviceVk* pDevice, const SamplerDesc & samplerDesc)
	: m_SamplerDesc(samplerDesc), m_pDevice(pDevice)
{
	Create();
}


BvSamplerVk::~BvSamplerVk()
{
	Destroy();
}


void BvSamplerVk::Create()
{
	VkSamplerCreateInfo samplerCreateInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
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
	//samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	//samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	bool usesCustomBorderColor = false;
	if (m_SamplerDesc.m_BorderColor[0] == 0.0f
		|| m_SamplerDesc.m_BorderColor[1] == 0.0f
		|| m_SamplerDesc.m_BorderColor[2] == 0.0f
		|| m_SamplerDesc.m_BorderColor[3] == 0.0f)
	{
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	}
	else if (m_SamplerDesc.m_BorderColor[0] == 0.0f
		|| m_SamplerDesc.m_BorderColor[1] == 0.0f
		|| m_SamplerDesc.m_BorderColor[2] == 0.0f
		|| m_SamplerDesc.m_BorderColor[3] == 1.0f)
	{
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	}
	else if (m_SamplerDesc.m_BorderColor[0] == 1.0f
		|| m_SamplerDesc.m_BorderColor[1] == 1.0f
		|| m_SamplerDesc.m_BorderColor[2] == 1.0f
		|| m_SamplerDesc.m_BorderColor[3] == 1.0f)
	{
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	}
	else
	{
		usesCustomBorderColor = true;
	}

	auto pDeviceInfo = m_pDevice->GetDeviceInfo();
	auto supportsBorderColor = pDeviceInfo->m_ExtendedFeatures.customBorderColorFeatures.customBorderColorWithoutFormat;
	bool supportsReduction = pDeviceInfo->m_DeviceFeatures1_2.samplerFilterMinmax;

	const void** pNext = &samplerCreateInfo.pNext;
	VkSamplerCustomBorderColorCreateInfoEXT customBorderColorCI{ VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT };
	if (usesCustomBorderColor && supportsBorderColor)
	{
		memcpy(customBorderColorCI.customBorderColor.float32, m_SamplerDesc.m_BorderColor, sizeof(m_SamplerDesc.m_BorderColor));
		//customBorderColorCI.format = VK_FORMAT_UNDEFINED;
		
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;
		*pNext = &customBorderColorCI;
		pNext = &customBorderColorCI.pNext;
	}

	VkSamplerReductionModeCreateInfo reductionCI{ VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO };
	if (m_SamplerDesc.m_ReductionMode != ReductionMode::kStandard && supportsReduction)
	{
		reductionCI.reductionMode = GetVkSamplerReductionMode(m_SamplerDesc.m_ReductionMode);
		*pNext = &reductionCI;
	}

	auto result = vkCreateSampler(m_pDevice->GetHandle(), &samplerCreateInfo, nullptr, &m_Sampler);
	if (result != VK_SUCCESS)
	{
		BvDebugVkResult(result);
	}
}


void BvSamplerVk::Destroy()
{
	if (m_Sampler)
	{
		vkDestroySampler(m_pDevice->GetHandle(), m_Sampler, nullptr);
		m_Sampler = VK_NULL_HANDLE;
	}
}
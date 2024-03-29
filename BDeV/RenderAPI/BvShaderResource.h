#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/Container/BvRobinMap.h"
#include "BDeV/Utils/BvEnum.h"
#include "BDeV/Utils/BvUtils.h"
#include "BvCommon.h"
#include "BDeV/Container/BvString.h"


class BvSampler;
class BvBufferView;
class BvTextureView;


enum class ShaderResourceType : u8
{
	kUnknown,
	kConstantBuffer,
	kStructuredBuffer,
	kRWStructuredBuffer,
	kFormattedBuffer,
	kRWFormattedBuffer,
	kTexture,
	kRWTexture,
	kSampler,
	kCombinedTextureSampler
};


struct ShaderResourceLayoutDesc
{
	struct ConstantDesc
	{
		BvString m_Name;
		u32 m_Size = 0;
		u32 m_Binding = 0;
		u32 m_SignatureSlot = 0;
		ShaderStage m_ShaderStages = ShaderStage::kAllGraphicsStages;
	};

	struct ResourceDesc
	{
		BvString m_Name;
		u32 m_Binding = 0;
		u32 m_Count = 0;
		ShaderResourceType m_ShaderResourceType = ShaderResourceType::kUnknown;
		ShaderStage m_ShaderStages = ShaderStage::kAllGraphicsStages;
	};

	struct StaticSamplerDesc
	{
		BvString m_Name;
		BvVector<BvSampler *> m_Samplers{};
		u32 m_Binding = 0;
		ShaderStage m_ShaderStages = ShaderStage::kAllGraphicsStages;
	};

	struct SetDesc
	{
		BvVector<ResourceDesc> m_ShaderResources{};
		BvVector<StaticSamplerDesc> m_StaticSamplers{};
		u32 m_SignatureIndex = 0;
	};

	void AddResource(ShaderResourceType resourceType, ShaderStage shaderStages, u32 binding,
		u32 set = 0, u32 count = 1)
	{
		AddResource("", resourceType, shaderStages, binding, set, count);
	}

	void AddResource(const char* pName, ShaderResourceType resourceType, ShaderStage shaderStages,
		u32 binding, u32 set = 0, u32 count = 1)
	{
		auto result = m_PerSetData.Emplace(set);
		bool alreadyExists = false;
		if (result.second)
		{
			result.first->second.m_SignatureIndex = static_cast<u32>(m_PerSetData.Size() + m_PushConstants.Size() - 1);
		}
		else
		{
			for (auto&& resource : result.first->second.m_ShaderResources)
			{
				if (resource.m_Binding == binding)
				{
					resource.m_ShaderStages |= shaderStages;
					alreadyExists = true;
					break;
				}
			}
		}

		if (!alreadyExists)
		{
			result.first->second.m_ShaderResources.PushBack({ pName, binding, count, resourceType, shaderStages });
		}
	}

	void AddStaticSampler(BvSampler** ppSamplers, ShaderStage shaderStages,
		u32 binding, u32 set = 0, u32 count = 1)
	{
		AddStaticSampler("", ppSamplers, shaderStages, binding, set, count);
	}

	void AddStaticSampler(const char* pName, BvSampler ** ppSamplers, ShaderStage shaderStages,
		u32 binding, u32 set = 0, u32 count = 1)
	{
		auto result = m_PerSetData.Emplace(set);
		bool alreadyExists = false;
		if (result.second)
		{
			result.first->second.m_SignatureIndex = static_cast<u32>(m_PerSetData.Size() + m_PushConstants.Size() - 1);
		}
		else
		{
			for (auto&& staticSampler : result.first->second.m_StaticSamplers)
			{
				if (staticSampler.m_Binding == binding)
				{
					staticSampler.m_ShaderStages |= shaderStages;
					alreadyExists = true;
					break;
				}
			}
		}

		if (!alreadyExists)
		{
			result.first->second.m_StaticSamplers.PushBack({ pName, std::move(BvVector<BvSampler *>(ppSamplers, ppSamplers + count)), binding, shaderStages });
		}
	}

	void AddPushConstant(ShaderStage shaderStages, u32 size, u32 binding = 0)
	{
		AddPushConstant("", shaderStages, size, binding);
	}

	void AddPushConstant(const char* pName, ShaderStage shaderStages, u32 size, u32 binding = 0)
	{
		bool alreadyExists = false;
		for (auto&& pushConstant : m_PushConstants)
		{
			// TODO: This needs some re-thinking, since on vulkan, push constants don't have a binding
			if (pushConstant.m_Binding == binding && pushConstant.m_Size == size)
			{
				pushConstant.m_ShaderStages |= shaderStages;
			}
		}

		if (!alreadyExists)
		{
			m_PushConstants.PushBack({ pName, size, binding, static_cast<u32>(m_PerSetData.Size() + m_PushConstants.Size()), shaderStages });
		}
	}

	BvRobinMap<u32, SetDesc> m_PerSetData;
	BvVector<ConstantDesc> m_PushConstants;
};


class BvShaderResourceLayout
{
	BV_NOCOPYMOVE(BvShaderResourceLayout);

public:
	BV_INLINE const ShaderResourceLayoutDesc & GetDesc() const { return m_ShaderResourceLayoutDesc; }

protected:
	BvShaderResourceLayout(const ShaderResourceLayoutDesc & shaderResourceLayoutDesc)
		: m_ShaderResourceLayoutDesc(shaderResourceLayoutDesc) {}
	virtual ~BvShaderResourceLayout() = 0 {}

protected:
	ShaderResourceLayoutDesc m_ShaderResourceLayoutDesc;
};


class BvShaderResourceParams
{
	BV_NOCOPYMOVE(BvShaderResourceParams);

public:
	virtual void SetBuffers(const u32 binding, const u32 count, const BvBufferView * const * const ppBuffers, const u32 startIndex = 0) = 0;
	virtual void SetTextures(const u32 binding, const u32 count, const BvTextureView * const * const ppTextures, const u32 startIndex = 0) = 0;
	virtual void SetSamplers(const u32 binding, const u32 count, const BvSampler * const * const ppSamplers, const u32 startIndex = 0) = 0;

	void SetBuffer(const u32 binding, const BvBufferView * const pBuffer) { SetBuffers(binding, 1, &pBuffer); }
	void SetTexture(const u32 binding, const BvTextureView * const pTexture) { SetTextures(binding, 1, &pTexture); }
	void SetSampler(const u32 binding, const BvSampler * const pSampler) { SetSamplers(binding, 1, &pSampler); }
	
	virtual void Update() = 0;

protected:
	BvShaderResourceParams() {}
	virtual ~BvShaderResourceParams() = 0 {}

protected:
};


enum class ShaderResourcePoolFlags : u8
{
	// No flags
	kNone = 0,

	// The default pool behaviour is to free the sets that are returned, but if
	// this flag is set, the sets will actually be recycled. In Vulkan, this
	// may be a more efficient path, since it won't trigger a different
	// allocation strategy (on some implementations).
	kRecycleDescriptors = 1
};
BV_USE_ENUM_CLASS_OPERATORS(ShaderResourcePoolFlags);


struct ShaderResourceSetPoolDesc
{
	static constexpr u32 kDefaultResourceCount = 1024;
	static constexpr u32 kDefaultSamplerResourceCount = 128;

	u32 m_ConstantBufferCount = kDefaultResourceCount;
	u32 m_StructuredBufferCount = kDefaultResourceCount;
	u32 m_RWStructuredBufferCount = kDefaultResourceCount;
	u32 m_FormattedBufferCount = kDefaultResourceCount;
	u32 m_RWFormattedBufferCount = kDefaultResourceCount;
	u32 m_TextureCount = kDefaultResourceCount;
	u32 m_RWTextureCount = kDefaultResourceCount;
	u32 m_SamplerCount = kDefaultSamplerResourceCount;
	ShaderResourcePoolFlags m_Flags = ShaderResourcePoolFlags::kNone;
};


class BvShaderResourceSetPool
{
	BV_NOCOPYMOVE(BvShaderResourceSetPool);

public:
	virtual void AllocateSets(u32 count, BvShaderResourceParams ** ppSets, const BvShaderResourceLayout * const pLayout, u32 set = 0) = 0;
	virtual void FreeSets(u32 count, BvShaderResourceParams ** ppSets) = 0;

	BvShaderResourceParams * AllocateSet(const BvShaderResourceLayout * const pLayout, u32 set = 0)
	{
		BvShaderResourceParams * pSet = nullptr;
		AllocateSets(1, &pSet, pLayout, set);

		return pSet;
	}

	void FreeSet(BvShaderResourceParams * & pSet)
	{
		FreeSets(1, &pSet);
	}

protected:
	BvShaderResourceSetPool(const ShaderResourceSetPoolDesc& poolDesc)
		: m_ShaderResourceSetPoolDesc(poolDesc) {}
	virtual ~BvShaderResourceSetPool() = 0 {}

protected:
	ShaderResourceSetPoolDesc m_ShaderResourceSetPoolDesc;
};
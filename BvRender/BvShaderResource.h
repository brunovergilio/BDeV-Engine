#pragma once


#include "BvCore/BvPlatform.h"
#include "BvCore/Container/BvVector.h"
#include "BvCore/Container/BvRobinMap.h"
#include "BvCore/Utils/BvEnum.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCommon.h"


class BvSampler;
class BvBufferView;
class BvTextureView;


enum class ShaderResourceType : u8
{
	kConstantBuffer,
	kStructuredBuffer,
	kRWStructuredBuffer,
	kFormattedBuffer,
	kRWFormattedBuffer,
	kTexture,
	kRWTexture,
	kSampler,
};


struct ShaderResourceLayoutDesc
{
	struct ConstantDesc
	{
		u32 m_Size = 0;
		u32 m_Binding = 0;
		u32 m_SignatureSlot = 0;
		ShaderStage m_ShaderStages = ShaderStage::kAll;
	};

	struct ResourceDesc
	{
		u32 m_Binding = 0;
		u32 m_Count = 0;
		ShaderResourceType m_ShaderResourceType = ShaderResourceType::kConstantBuffer;
		ShaderStage m_ShaderStages = ShaderStage::kAll;
	};

	struct StaticSamplerDesc
	{
		BvVector<BvSampler *> m_Samplers{};
		u32 m_Binding = 0;
		ShaderStage m_ShaderStages = ShaderStage::kAll;
	};

	struct SetDesc
	{
		BvVector<ResourceDesc> m_ShaderResources{};
		BvVector<StaticSamplerDesc> m_StaticSamplers{};
		u32 m_SignatureIndex = 0;
	};

	void AddResource(const ShaderResourceType resourceType, const ShaderStage shaderStages, const u32 binding,
		const u32 set = 0, const u32 count = 1)
	{
		auto result = m_PerSetData.Emplace(set);
		if (result.second)
		{
			result.first->second.m_SignatureIndex = static_cast<u32>(m_PerSetData.Size() + m_PushConstants.Size() - 1);
		}
		result.first->second.m_ShaderResources.PushBack({ binding, count, resourceType, shaderStages });
	}

	void AddStaticSampler(BvSampler ** ppSamplers, const ShaderStage shaderStages,
		const u32 binding, const u32 set = 0, const u32 count = 1)
	{
		auto result = m_PerSetData.Emplace(set);
		if (result.second)
		{
			result.first->second.m_SignatureIndex = static_cast<u32>(m_PerSetData.Size() + m_PushConstants.Size() - 1);
		}
		result.first->second.m_StaticSamplers.PushBack({ std::move(BvVector<BvSampler *>(ppSamplers, ppSamplers + count)), binding, shaderStages });
	}

	void AddPushConstant(const ShaderStage shaderStages, const u32 size, const u32 binding = 0)
	{
		m_PushConstants.PushBack({ size, binding, static_cast<u32>(m_PerSetData.Size() + m_PushConstants.Size()), shaderStages });
	}

	BV_INLINE const BvRobinMap<u32, SetDesc> & GetSetData() const { return m_PerSetData; }
	BV_INLINE const BvVector<ConstantDesc> & GetPushConstants() const { return m_PushConstants; }

private:
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


class BvShaderResourceSet
{
	BV_NOCOPYMOVE(BvShaderResourceSet);

public:
	virtual void SetBuffers(const u32 binding, const u32 count, const BvBufferView * const * const ppBuffers) = 0;
	virtual void SetTextures(const u32 binding, const u32 count, const BvTextureView * const * const ppTextures) = 0;
	virtual void SetSamplers(const u32 binding, const u32 count, const BvSampler * const * const ppSamplers) = 0;

	void SetBuffer(const u32 binding, const BvBufferView * const pBuffer) { SetBuffers(binding, 1, &pBuffer); }
	void SetTexture(const u32 binding, const BvTextureView * const pTexture) { SetTextures(binding, 1, &pTexture); }
	void SetSampler(const u32 binding, const BvSampler * const pSampler) { SetSamplers(binding, 1, &pSampler); }
	
	virtual void Update() = 0;

protected:
	BvShaderResourceSet() {}
	virtual ~BvShaderResourceSet() = 0 {}

protected:
};


struct ShaderResourceSetPoolDesc
{
	static constexpr u32 kDefaultResourceCount = 1024;

	u32 m_ConstantBufferCount = kDefaultResourceCount;
	u32 m_StructuredBufferCount = kDefaultResourceCount;
	u32 m_RWStructuredBufferCount = kDefaultResourceCount;
	u32 m_FormattedBufferCount = kDefaultResourceCount;
	u32 m_RWFormattedBufferCount = kDefaultResourceCount;
	u32 m_TextureCount = kDefaultResourceCount;
	u32 m_RWTextureCount = kDefaultResourceCount;
	u32 m_SamplerCount = kDefaultResourceCount;
};


class BvShaderResourceSetPool
{
	BV_NOCOPYMOVE(BvShaderResourceSetPool);

public:
	virtual void AllocateSets(const u32 count, BvShaderResourceSet ** ppSets, const BvShaderResourceLayout * const pLayout, const u32 set = 0) = 0;
	virtual void FreeSets(const u32 count, BvShaderResourceSet ** ppSets) = 0;

	BvShaderResourceSet * AllocateSet(const BvShaderResourceLayout * const pLayout, const u32 set = 0)
	{
		BvShaderResourceSet * pSet = nullptr;
		AllocateSets(1, &pSet, pLayout, set);

		return pSet;
	}

	void FreeSet(BvShaderResourceSet * & pSet)
	{
		FreeSets(1, &pSet);
	}

protected:
	BvShaderResourceSetPool() {}
	virtual ~BvShaderResourceSetPool() = 0 {}
};
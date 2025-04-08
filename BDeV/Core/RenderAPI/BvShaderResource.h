#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BvRenderCommon.h"
#include "BDeV/Core/Container/BvStringId.h"
#include "BDeV/Core/Utils/BvObject.h"


class IBvSampler;
class IBvBufferView;
class IBvTextureView;


// HLSL Type					DirectX Descriptor Type		Vulkan Descriptor Type		GLSL Type
// SamplerState					Sampler						Sampler						uniform sampler*
// SamplerComparisonState		Sampler						Sampler						uniform sampler*Shadow
// Buffer						SRV							Uniform Texel Buffer		uniform samplerBuffer
// RWBuffer						UAV							Storage Texel Buffer		uniform imageBuffer
// Texture*						SRV							Sampled Image				uniform texture*
// RWTexture*					UAV							Storage Image				uniform image*
// cbuffer						CBV							Uniform Buffer				uniform{ ... }
// ConstantBuffer				CBV							Uniform Buffer				uniform{ ... }
// tbuffer						SRV							Uniform Texel Buffer		uniform samplerBuffer
// TextureBuffer				SRV							Uniform Texel Buffer		uniform samplerBuffer
// StructuredBuffer				SRV							Storage Buffer				buffer{ ... }
// RWStructuredBuffer			UAV							Storage Buffer				buffer{ ... }
// ByteAddressBuffer			SRV							Storage Buffer				-
// RWByteAddressBuffer			UAV							Storage Buffer				-
// AppendStructuredBuffer		UAV							Storage Buffer				-
// ConsumeStructuredBuffer		UAV							Storage Buffer				-


struct ShaderResourceDesc
{
	BvStringId m_Name;
	u32 m_Binding;
	u32 m_Count;
	ShaderResourceType m_ShaderResourceType;
	ShaderStage m_ShaderStages;
	const IBvSampler* const* m_ppStaticSamplers;
	bool m_Bindless;

	template<ShaderResourceType Type>
	static ShaderResourceDesc As(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const IBvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ name, binding, 0, count, Type, shaderStages, nullptr }; }
	template<ShaderResourceType Type>
	static ShaderResourceDesc As(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const IBvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ BvStringId::Empty(), binding, 0, count, Type, shaderStages, nullptr}; }

	static ShaderResourceDesc AsConstantBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kConstantBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsStructuredBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kStructuredBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsRWStructuredBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kRWStructuredBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsDynamicConstantBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kDynamicConstantBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsDynamicStructuredBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kDynamicStructuredBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsDynamicRWStructuredBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kDynamicRWStructuredBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsFormattedBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kFormattedBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsRWFormattedBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kRWFormattedBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsTexture(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kTexture, shaderStages, nullptr }; }
	static ShaderResourceDesc AsRWTexture(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kRWTexture, shaderStages, nullptr }; }
	static ShaderResourceDesc AsSampler(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kSampler, shaderStages, nullptr }; }
	static ShaderResourceDesc AsInputAttachment(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kInputAttachment, shaderStages, nullptr }; }
	static ShaderResourceDesc AsStaticSampler(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const IBvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ name, binding, count, ShaderResourceType::kSampler, shaderStages, ppSamplers }; }

	static ShaderResourceDesc AsConstantBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsConstantBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsStructuredBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsRWStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWStructuredBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsDynamicConstantBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsDynamicConstantBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsDynamicStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsDynamicStructuredBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsDynamicRWStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsDynamicRWStructuredBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsFormattedBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsFormattedBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsRWFormattedBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWFormattedBuffer(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsTexture(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsTexture(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsRWTexture(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWTexture(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsSampler(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsSampler(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsInputAttachment(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsInputAttachment(BvStringId::Empty(), binding, shaderStages, count); }
	static ShaderResourceDesc AsStaticSampler(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const IBvSampler* const* ppSamplers = nullptr) { return AsStaticSampler(BvStringId::Empty(), binding, shaderStages, count, ppSamplers); }

	friend bool operator<(const ShaderResourceDesc& lhs, const ShaderResourceDesc& rhs)
	{
		return lhs.m_Binding < rhs.m_Binding;
	}
};


struct ShaderResourceConstantDesc
{
	BvStringId m_Name;
	u32 m_Binding;
	u32 m_Size;
	ShaderStage m_ShaderStages;

	template<typename T>
	static ShaderResourceConstantDesc As(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return { name, binding, sizeof(T), shaderStages}; }
	template<typename T>
	static ShaderResourceConstantDesc As(const BvStringId name, ShaderStage shaderStages = ShaderStage::kAllStages) { return { name, 0, sizeof(T), shaderStages }; }
	template<typename T>
	static ShaderResourceConstantDesc As(ShaderStage shaderStages = ShaderStage::kAllStages) { return { nullptr, 0, sizeof(T), shaderStages }; }

	friend bool operator<(const ShaderResourceConstantDesc& lhs, const ShaderResourceConstantDesc& rhs)
	{
		return lhs.m_Binding < rhs.m_Binding;
	}
};


struct ShaderResourceSetDesc
{
	u32 m_Index;
	u32 m_ResourceCount;
	const ShaderResourceDesc* m_pResources;
	bool m_Bindless;
	u32 m_ConstantCount;
	const ShaderResourceConstantDesc* m_pConstants;

	friend bool operator<(const ShaderResourceSetDesc& lhs, const ShaderResourceSetDesc& rhs)
	{
		return lhs.m_Index < rhs.m_Index;
	}
};


struct ShaderResourceLayoutDesc
{
	u32 m_ShaderResourceSetCount = 0;
	const ShaderResourceSetDesc* m_pShaderResourceSets = nullptr;
};


//BV_OBJECT_DEFINE_ID(IBvShaderResourceLayout, "79547e0c-dc23-4354-9a51-c1af70d20b83");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvShaderResourceLayout);
class IBvShaderResourceLayout : public BvRCObj
{
	BV_NOCOPYMOVE(IBvShaderResourceLayout);

public:
	virtual const ShaderResourceLayoutDesc& GetDesc() const = 0;

protected:
	IBvShaderResourceLayout() {}
	~IBvShaderResourceLayout() {}
};


class IBvShaderResourceParams
{
	BV_NOCOPYMOVE(IBvShaderResourceParams);

public:
	virtual void SetConstantBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWStructuredBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWFormattedBuffers(u32 count, const IBvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetTextures(u32 count, const IBvTextureView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWTextures(u32 count, const IBvTextureView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetSamplers(u32 count, const IBvSampler* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetAccelerationStructures(u32 count, const IBvAccelerationStructure* const* ppResources, u32 binding, u32 startIndex = 0) = 0;

	BV_INLINE void SetConstantBuffer(const IBvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetConstantBuffers(1, &pResource, binding, startIndex); }
	BV_INLINE void SetStructuredBuffer(const IBvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetStructuredBuffers(1, &pResource, binding, startIndex); }
	BV_INLINE void SetRWStructuredBuffer(const IBvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetRWStructuredBuffers(1, &pResource, binding, startIndex); }
	BV_INLINE void SetFormattedBuffer(const IBvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetFormattedBuffers(1, &pResource, binding, startIndex); }
	BV_INLINE void SetRWFormattedBuffer(const IBvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetRWFormattedBuffers(1, &pResource, binding, startIndex); }
	BV_INLINE void SetTexture(const IBvTextureView* pResource, u32 binding, u32 startIndex = 0) { SetTextures(1, &pResource, binding, startIndex); }
	BV_INLINE void SetRWTexture(const IBvTextureView* pResource, u32 binding, u32 startIndex = 0) { SetRWTextures(1, &pResource, binding, startIndex); }
	BV_INLINE void SetSampler(const IBvSampler* pResource, u32 binding, u32 startIndex = 0) { SetSamplers(1, &pResource, binding, startIndex); }
	BV_INLINE void SetAccelerationStructure(const IBvAccelerationStructure* pResource, u32 binding, u32 startIndex = 0) { SetAccelerationStructures(1, &pResource, binding, startIndex); }
	
	virtual void Bind() = 0;

protected:
	IBvShaderResourceParams() {}
	~IBvShaderResourceParams() {}
};
#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BvRenderCommon.h"
#include "BDeV/Core/Container/BvStringId.h"
#include "BvRenderDeviceObject.h"


class BvSampler;
class BvBufferView;
class BvTextureView;


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
	u32 m_Set;
	u32 m_Count;
	ShaderResourceType m_ShaderResourceType;
	ShaderStage m_ShaderStages;
	const BvSampler* const* m_ppStaticSamplers;
	bool m_Bindless;

	template<ShaderResourceType Type>
	static ShaderResourceDesc As(const BvStringId name, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ name, binding, set, count, Type, shaderStages, nullptr }; }
	template<ShaderResourceType Type>
	static ShaderResourceDesc As(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ name, binding, 0, count, Type, shaderStages, nullptr }; }
	template<ShaderResourceType Type>
	static ShaderResourceDesc As(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ nullptr, binding, set, count, Type, shaderStages, nullptr}; }
	template<ShaderResourceType Type>
	static ShaderResourceDesc As(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ nullptr, binding, 0, count, Type, shaderStages, nullptr }; }

	static ShaderResourceDesc AsConstantBuffer(const BvStringId name, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, set, count, ShaderResourceType::kConstantBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsStructuredBuffer(const BvStringId name, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, set, count, ShaderResourceType::kStructuredBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsRWStructuredBuffer(const BvStringId name, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, set, count, ShaderResourceType::kRWStructuredBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsFormattedBuffer(const BvStringId name, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, set, count, ShaderResourceType::kFormattedBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsRWFormattedBuffer(const BvStringId name, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, set, count, ShaderResourceType::kRWFormattedBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsTexture(const BvStringId name, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, set, count, ShaderResourceType::kTexture, shaderStages, nullptr }; }
	static ShaderResourceDesc AsRWTexture(const BvStringId name, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, set, count, ShaderResourceType::kRWTexture, shaderStages, nullptr }; }
	static ShaderResourceDesc AsSampler(const BvStringId name, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ name, binding, set, count, ShaderResourceType::kSampler, shaderStages, nullptr }; }
	static ShaderResourceDesc AsStaticSampler(const BvStringId name, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ name, binding, set, count, ShaderResourceType::kSampler, shaderStages, ppSamplers }; }

	static ShaderResourceDesc AsConstantBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsConstantBuffer(name, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsStructuredBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsStructuredBuffer(name, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsRWStructuredBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWStructuredBuffer(name, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsFormattedBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsFormattedBuffer(name, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsRWFormattedBuffer(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWFormattedBuffer(name, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsTexture(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsTexture(name, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsRWTexture(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWTexture(name, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsSampler(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsSampler(name, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsStaticSampler(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return AsStaticSampler(name, binding, 0, shaderStages, count, ppSamplers); }

	static ShaderResourceDesc AsConstantBuffer(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsConstantBuffer(BvStringId::Empty(), binding, set, shaderStages, count); }
	static ShaderResourceDesc AsStructuredBuffer(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsStructuredBuffer(BvStringId::Empty(), binding, set, shaderStages, count); }
	static ShaderResourceDesc AsRWStructuredBuffer(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWStructuredBuffer(BvStringId::Empty(), binding, set, shaderStages, count); }
	static ShaderResourceDesc AsFormattedBuffer(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsFormattedBuffer(BvStringId::Empty(), binding, set, shaderStages, count); }
	static ShaderResourceDesc AsRWFormattedBuffer(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWFormattedBuffer(BvStringId::Empty(), binding, set, shaderStages, count); }
	static ShaderResourceDesc AsTexture(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsTexture(BvStringId::Empty(), binding, set, shaderStages, count); }
	static ShaderResourceDesc AsRWTexture(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWTexture(BvStringId::Empty(), binding, set, shaderStages, count); }
	static ShaderResourceDesc AsSampler(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsSampler(BvStringId::Empty(), binding, set, shaderStages, count); }
	static ShaderResourceDesc AsStaticSampler(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return AsStaticSampler(BvStringId::Empty(), binding, set, shaderStages, count, ppSamplers); }

	static ShaderResourceDesc AsConstantBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsConstantBuffer(BvStringId::Empty(), binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsStructuredBuffer(BvStringId::Empty(), binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsRWStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWStructuredBuffer(BvStringId::Empty(), binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsFormattedBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsFormattedBuffer(BvStringId::Empty(), binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsRWFormattedBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWFormattedBuffer(BvStringId::Empty(), binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsTexture(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsTexture(BvStringId::Empty(), binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsRWTexture(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWTexture(BvStringId::Empty(), binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsSampler(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsSampler(BvStringId::Empty(), binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsStaticSampler(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return AsStaticSampler(BvStringId::Empty(), binding, 0, shaderStages, count, ppSamplers); }
};


struct ShaderResourceConstantDesc
{
	BvStringId m_Name;
	u32 m_Binding;
	u32 m_Set;
	u32 m_Size;
	ShaderStage m_ShaderStages;

	template<typename T>
	static ShaderResourceConstantDesc As(const BvStringId name, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages) { return { name, binding, set, sizeof(T), shaderStages}; }
	template<typename T>
	static ShaderResourceConstantDesc As(const BvStringId name, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return { name, binding, 0, sizeof(T), shaderStages}; }
	template<typename T>
	static ShaderResourceConstantDesc As(const BvStringId name, ShaderStage shaderStages = ShaderStage::kAllStages) { return { name, 0, 0, sizeof(T), shaderStages }; }
	template<typename T>
	static ShaderResourceConstantDesc As(ShaderStage shaderStages = ShaderStage::kAllStages) { return { nullptr, 0, sizeof(T), shaderStages }; }
};


struct ShaderResourceSetDesc
{
	u32 m_Index;
	u32 m_ResourceCount;
	const ShaderResourceDesc* m_pResources;
	bool m_Bindless;
};


struct ShaderResourceLayoutDesc
{
	BvRobinMap<u32, BvRobinMap<u32, ShaderResourceDesc>> m_ShaderResources;
	ShaderResourceConstantDesc m_ShaderResourceConstant;
	BvVector<const BvSampler*> m_Samplers;
};


BV_OBJECT_DEFINE_ID(BvShaderResourceLayout, "79547e0c-dc23-4354-9a51-c1af70d20b83");
class BvShaderResourceLayout : public IBvRenderDeviceObject
{
	BV_NOCOPYMOVE(BvShaderResourceLayout);

public:
	BV_INLINE const ShaderResourceLayoutDesc& GetDesc() const { return m_ShaderResourceLayoutDesc; }

protected:
	BvShaderResourceLayout() {}
	virtual ~BvShaderResourceLayout() = 0 {}

protected:
	ShaderResourceLayoutDesc m_ShaderResourceLayoutDesc;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvShaderResourceLayout);


class BvShaderResourceParams
{
	BV_NOCOPYMOVE(BvShaderResourceParams);

public:
	virtual void SetConstantBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetStructuredBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWStructuredBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetFormattedBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWFormattedBuffers(u32 count, const BvBufferView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetTextures(u32 count, const BvTextureView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetRWTextures(u32 count, const BvTextureView* const* ppResources, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetSamplers(u32 count, const BvSampler* const* ppResources, u32 binding, u32 startIndex = 0) = 0;

	void SetConstantBuffer(u32 count, const BvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetConstantBuffers(1, &pResource, binding, startIndex); }
	void SetStructuredBuffer(u32 count, const BvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetStructuredBuffers(1, &pResource, binding, startIndex); }
	void SetRWStructuredBuffer(u32 count, const BvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetRWStructuredBuffers(1, &pResource, binding, startIndex); }
	void SetFormattedBuffer(u32 count, const BvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetFormattedBuffers(1, &pResource, binding, startIndex); }
	void SetRWFormattedBuffer(u32 count, const BvBufferView* pResource, u32 binding, u32 startIndex = 0) { SetRWFormattedBuffers(1, &pResource, binding, startIndex); }
	void SetTexture(u32 count, const BvTextureView* pResource, u32 binding, u32 startIndex = 0) { SetTextures(1, &pResource, binding, startIndex); }
	void SetRWTexture(u32 count, const BvTextureView* pResource, u32 binding, u32 startIndex = 0) { SetRWTextures(1, &pResource, binding, startIndex); }
	void SetSampler(u32 count, const BvSampler* pResource, u32 binding, u32 startIndex = 0) { SetSamplers(1, &pResource, binding, startIndex); }
	
	virtual void Bind() = 0;

protected:
	BvShaderResourceParams() {}
	virtual ~BvShaderResourceParams() = 0 {}
};
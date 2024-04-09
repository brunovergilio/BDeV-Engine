#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvVector.h"
#include "BDeV/Container/BvRobinMap.h"
#include "BDeV/Utils/BvEnum.h"
#include "BDeV/Utils/BvUtils.h"
#include "BvCommon.h"
#include "BDeV/Container/BvString.h"
#include "BDeV/Container/BvStringId.h"


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
// tbuffer						CBV							Storage Buffer				-
// TextureBuffer				CBV							Storage Buffer				-
// StructuredBuffer				SRV							Storage Buffer				buffer{ ... }
// RWStructuredBuffer			UAV							Storage Buffer				buffer{ ... }
// ByteAddressBuffer			SRV							Storage Buffer				-
// RWByteAddressBuffer			UAV							Storage Buffer				-
// AppendStructuredBuffer		UAV							Storage Buffer				-
// ConsumeStructuredBuffer		UAV							Storage Buffer				-


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
};


struct ShaderResourceDesc
{
	BvStringId m_Name;
	u32 m_Binding;
	u32 m_Set;
	u32 m_Count;
	ShaderResourceType m_ShaderResourceType;
	ShaderStage m_ShaderStages;
	const BvSampler* const* m_ppStaticSamplers;

	template<ShaderResourceType Type>
	static ShaderResourceDesc As(const char* pName, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ pName, binding, set, count, Type, shaderStages, nullptr }; }
	template<ShaderResourceType Type>
	static ShaderResourceDesc As(const char* pName, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ pName, binding, 0, count, Type, shaderStages, nullptr }; }
	template<ShaderResourceType Type>
	static ShaderResourceDesc As(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ nullptr, binding, set, count, Type, shaderStages, nullptr}; }
	template<ShaderResourceType Type>
	static ShaderResourceDesc As(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ nullptr, binding, 0, count, Type, shaderStages, nullptr }; }

	static ShaderResourceDesc AsConstantBuffer(const char* pName, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ pName, binding, set, count, ShaderResourceType::kConstantBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsStructuredBuffer(const char* pName, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ pName, binding, set, count, ShaderResourceType::kStructuredBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsRWStructuredBuffer(const char* pName, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ pName, binding, set, count, ShaderResourceType::kRWStructuredBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsFormattedBuffer(const char* pName, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ pName, binding, set, count, ShaderResourceType::kFormattedBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsRWFormattedBuffer(const char* pName, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ pName, binding, set, count, ShaderResourceType::kRWFormattedBuffer, shaderStages, nullptr }; }
	static ShaderResourceDesc AsTexture(const char* pName, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ pName, binding, set, count, ShaderResourceType::kTexture, shaderStages, nullptr }; }
	static ShaderResourceDesc AsRWTexture(const char* pName, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ pName, binding, set, count, ShaderResourceType::kRWTexture, shaderStages, nullptr }; }
	static ShaderResourceDesc AsSampler(const char* pName, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return ShaderResourceDesc{ pName, binding, set, count, ShaderResourceType::kSampler, shaderStages, nullptr }; }
	static ShaderResourceDesc AsStaticSampler(const char* pName, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return ShaderResourceDesc{ pName, binding, set, count, ShaderResourceType::kSampler, shaderStages, ppSamplers }; }

	static ShaderResourceDesc AsConstantBuffer(const char* pName, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsConstantBuffer(pName, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsStructuredBuffer(const char* pName, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsStructuredBuffer(pName, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsRWStructuredBuffer(const char* pName, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWStructuredBuffer(pName, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsFormattedBuffer(const char* pName, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsFormattedBuffer(pName, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsRWFormattedBuffer(const char* pName, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWFormattedBuffer(pName, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsTexture(const char* pName, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsTexture(pName, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsRWTexture(const char* pName, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWTexture(pName, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsSampler(const char* pName, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsSampler(pName, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsStaticSampler(const char* pName, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return AsStaticSampler(pName, binding, 0, shaderStages, count, ppSamplers); }

	static ShaderResourceDesc AsConstantBuffer(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsConstantBuffer(nullptr, binding, set, shaderStages, count); }
	static ShaderResourceDesc AsStructuredBuffer(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsStructuredBuffer(nullptr, binding, set, shaderStages, count); }
	static ShaderResourceDesc AsRWStructuredBuffer(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWStructuredBuffer(nullptr, binding, set, shaderStages, count); }
	static ShaderResourceDesc AsFormattedBuffer(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsFormattedBuffer(nullptr, binding, set, shaderStages, count); }
	static ShaderResourceDesc AsRWFormattedBuffer(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWFormattedBuffer(nullptr, binding, set, shaderStages, count); }
	static ShaderResourceDesc AsTexture(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsTexture(nullptr, binding, set, shaderStages, count); }
	static ShaderResourceDesc AsRWTexture(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWTexture(nullptr, binding, set, shaderStages, count); }
	static ShaderResourceDesc AsSampler(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsSampler(nullptr, binding, set, shaderStages, count); }
	static ShaderResourceDesc AsStaticSampler(u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return AsStaticSampler(nullptr, binding, set, shaderStages, count, ppSamplers); }

	static ShaderResourceDesc AsConstantBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsConstantBuffer(nullptr, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsStructuredBuffer(nullptr, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsRWStructuredBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWStructuredBuffer(nullptr, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsFormattedBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsFormattedBuffer(nullptr, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsRWFormattedBuffer(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWFormattedBuffer(nullptr, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsTexture(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsTexture(nullptr, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsRWTexture(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsRWTexture(nullptr, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsSampler(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1) { return AsSampler(nullptr, binding, 0, shaderStages, count); }
	static ShaderResourceDesc AsStaticSampler(u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages, u32 count = 1, const BvSampler* const* ppSamplers = nullptr) { return AsStaticSampler(nullptr, binding, 0, shaderStages, count, ppSamplers); }
};

struct ShaderResourceConstantDesc
{
	BvStringId m_Name;
	u32 m_Binding;
	u32 m_Set;
	u32 m_Size;
	ShaderStage m_ShaderStages;

	template<typename T>
	static ShaderResourceConstantDesc As(const char* pName, u32 binding, u32 set, ShaderStage shaderStages = ShaderStage::kAllStages) { return { pName, binding, set, sizeof(T), shaderStages}; }
	template<typename T>
	static ShaderResourceConstantDesc As(const char* pName, u32 binding, ShaderStage shaderStages = ShaderStage::kAllStages) { return { pName, binding, 0, sizeof(T), shaderStages}; }
	template<typename T>
	static ShaderResourceConstantDesc As(const char* pName, ShaderStage shaderStages = ShaderStage::kAllStages) { return { pName, 0, 0, sizeof(T), shaderStages }; }
	template<typename T>
	static ShaderResourceConstantDesc As(ShaderStage shaderStages = ShaderStage::kAllStages) { return { nullptr, 0, sizeof(T), shaderStages }; }
};


struct ShaderResourceLayoutDesc
{
	BvRobinMap<u32, BvRobinMap<u32, ShaderResourceDesc>> m_ShaderResources;
	ShaderResourceConstantDesc m_ShaderResourceConstant;
	BvVector<const BvSampler*> m_Samplers;
};


class BvShaderResourceLayout
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


class BvShaderResourceParams
{
	BV_NOCOPYMOVE(BvShaderResourceParams);

public:
	virtual void SetBuffers(u32 count, const BvBufferView* const* ppBuffers, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetTextures(u32 count, const BvTextureView* const* ppTextures, u32 binding, u32 startIndex = 0) = 0;
	virtual void SetSamplers(u32 count, const BvSampler* const* ppSamplers, u32 binding, u32 startIndex = 0) = 0;

	void SetBuffer(const BvBufferView* pBuffer, u32 binding, u32 startIndex = 0) { SetBuffers(1, &pBuffer, binding, startIndex); }
	void SetTexture(const BvTextureView* pTexture, u32 binding, u32 startIndex = 0) { SetTextures(1, &pTexture, binding, startIndex); }
	void SetSampler(const BvSampler* pSampler, u32 binding, u32 startIndex = 0) { SetSamplers(1, &pSampler, binding, startIndex); }
	
	virtual void Update() = 0;

protected:
	BvShaderResourceParams() {}
	virtual ~BvShaderResourceParams() = 0 {}
};
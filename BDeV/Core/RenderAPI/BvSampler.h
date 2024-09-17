#pragma once


#include "BvRenderCommon.h"
#include "BDeV/Core/Utils/BvUtils.h"


enum class Filter : u8
{
	kPoint,
	kLinear,
};


enum class MipMapFilter : u8
{
	kPoint,
	kLinear,
};


enum class AddressMode : u8
{
	kWrap,
	kMirror,
	kClamp,
	kBorder,
	kMirrorOnce,
};


struct SamplerDesc
{
	Filter			m_MagFilter = Filter::kLinear;
	Filter			m_MinFilter = Filter::kLinear;
	MipMapFilter	m_MipmapMode = MipMapFilter::kLinear;
	AddressMode		m_AddressModeU = AddressMode::kWrap;
	AddressMode		m_AddressModeV = AddressMode::kWrap;
	AddressMode		m_AddressModeW = AddressMode::kWrap;
	bool			m_CompareEnable = false;
	CompareOp		m_CompareOp = CompareOp::kNever;
	bool			m_AnisotropyEnable = false;
	float			m_MaxAnisotropy = 1.0f;
	float			m_MipLodBias = 0.0f;
	float			m_MinLod = 0.0f;
	float			m_MaxLod = 1.0f;
	float			m_BorderColor[4]{};
};



class BvSampler : public IBvRenderDeviceChild
{
	BV_NOCOPYMOVE(BvSampler);

public:
	BV_INLINE const SamplerDesc & GetDesc() const { return m_SamplerDesc; }

protected:
	BvSampler(const SamplerDesc & samplerDesc)
		: m_SamplerDesc(samplerDesc) {}
	virtual ~BvSampler() = 0 {}

protected:
	SamplerDesc m_SamplerDesc;
};
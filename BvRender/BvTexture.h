#pragma once


#include "BvRender/BvCommon.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCore/Utils/BvEnum.h"


enum class TextureType : u8
{
	kTexture1D,
	kTexture2D,
	kTexture3D,
};


enum class TextureUsage : u8
{
	kDefault = 0x0,
	kTransferSrc = 0x1,
	kTransferDst = 0x2,
	kShaderResource = 0x4,
	kUnorderedAccess = 0x8,
	kColorTarget = 0x10,
	kDepthStencilTarget = 0x20,
};
BV_USE_ENUM_CLASS_OPERATORS(TextureUsage);


struct TextureDesc
{
	Extent m_Size{ 1,1,1 };
	u32 m_Alignment = 0;
	u8 m_MipLevels = 1;
	u8 m_SampleCount = 1;
	TextureType m_ImageType = TextureType::kTexture2D;
	Format m_Format = Format::kUndefined;
	TextureUsage m_UsageFlags = TextureUsage::kDefault;
	MemoryFlags m_MemoryFlags = MemoryFlags::kDeviceLocal;
	bool m_UseAsCubeMap = false;
	bool m_UseForStenciling = false;
};


class BvTexture
{
	BV_NOCOPYMOVE(BvTexture);

public:
	enum class ClassType : u8
	{
		kTexture,
		kSwapChainTexture
	};

	BV_INLINE const TextureDesc & GetDesc() const { return m_TextureDesc; }

	virtual ClassType GetClassType() const = 0;

protected:
	BvTexture(const TextureDesc & textureDesc)
		: m_TextureDesc(textureDesc) {}
	virtual ~BvTexture() = 0 {}

protected:
	TextureDesc m_TextureDesc;
};
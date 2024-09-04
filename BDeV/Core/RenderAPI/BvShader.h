#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/Container/BvVector.h"


class BvShader
{
	BV_NOCOPYMOVE(BvShader);

public:
	BV_INLINE ShaderStage GetShaderStage() const { return m_ShaderStage; }
	BV_INLINE const BvVector<u8>& GetShaderBlob() const { return m_ShaderBlob; }
	BV_INLINE const char* GetEntryPoint() const { return m_pEntryPoint; }

protected:
	BvShader() {}
	virtual ~BvShader() {}

protected:
	BvVector<u8> m_ShaderBlob{};
	ShaderStage m_ShaderStage = ShaderStage::kUnknown;
	const char* m_pEntryPoint = "main";
};
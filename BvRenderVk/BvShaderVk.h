#pragma once


#include "BDeV/Core/RenderAPI/BvShader.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvShaderVk final : public IBvShader, public IBvResourceVk
{
public:
	BvShaderVk(BvRenderDeviceVk* pDevice, const ShaderDesc& shaderDesc);
	~BvShaderVk();

	BV_INLINE ShaderStage GetShaderStage() const override { return m_ShaderStage; }
	BV_INLINE const BvVector<u8>& GetShaderBlob() const override { return m_ShaderBlob; }
	BV_INLINE const char* GetEntryPoint() const override { return m_pEntryPoint; }

private:
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvVector<u8> m_ShaderBlob;
	ShaderStage m_ShaderStage = ShaderStage::kUnknown;
	const char* m_pEntryPoint = "main";
};
BV_OBJECT_DEFINE_ID(BvShaderVk, "73dd3e67-1619-4f36-a29e-99bf0f90ea0b");
BV_CREATE_CAST_TO_VK(BvShader)
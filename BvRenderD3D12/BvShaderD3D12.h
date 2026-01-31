#pragma once


#include "BDeV/Core/RenderAPI/BvShader.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


class BvShaderD3D12 final : public IBvShader, public IBvResourceD3D12
{
public:
	BvShaderD3D12(BvRenderDeviceD3D12* pDevice, const ShaderDesc& shaderDesc);
	~BvShaderD3D12();

	BV_INLINE ShaderStage GetShaderStage() const override { return m_ShaderStage; }
	BV_INLINE const BvVector<u8>& GetShaderBlob() const override { return m_ShaderBlob; }
	BV_INLINE const char* GetEntryPoint() const override { return m_pEntryPoint; }
	BV_INLINE bool IsValid() const { return m_ShaderBlob.Size() > 0; }

private:
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	BvVector<u8> m_ShaderBlob{};
	ShaderStage m_ShaderStage = ShaderStage::kUnknown;
	const char* m_pEntryPoint = "main";
};
BV_OBJECT_DEFINE_ID(BvShaderD3D12, "73dd3e67-1619-4f36-a29e-99bf0f90ea0b");
BV_CREATE_CAST_TO_D3D12(BvShader)
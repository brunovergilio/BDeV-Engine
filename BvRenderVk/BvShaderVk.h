#pragma once


#include "BDeV/Core/RenderAPI/BvShader.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(IBvShaderVk, "73dd3e67-1619-4f36-a29e-99bf0f90ea0b");
class IBvShaderVk : public IBvShader
{
	BV_NOCOPYMOVE(IBvShaderVk);

public:
	virtual bool IsValid() const = 0;

protected:
	IBvShaderVk() {}
	~IBvShaderVk() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvShaderVk);


class BvShaderVk final : public IBvShaderVk
{
	BV_NOCOPYMOVE(BvShaderVk);

public:
	BvShaderVk(BvRenderDeviceVk* pDevice, const ShaderCreateDesc& shaderCreateDesc);
	~BvShaderVk();

	IBvRenderDevice* GetDevice() override;
	BV_INLINE ShaderStage GetShaderStage() const override { return m_ShaderStage; }
	BV_INLINE const BvVector<u8>& GetShaderBlob() const override { return m_ShaderBlob; }
	BV_INLINE const char* GetEntryPoint() const override { return m_pEntryPoint; }
	BV_INLINE bool IsValid() const override { return m_ShaderBlob.Size() > 0; }

	BV_OBJECT_IMPL_INTERFACE(IBvShaderVk, IBvShader, IBvRenderDeviceObject);

private:
	void Compile(const ShaderCreateDesc& shaderCreateDesc);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	BvVector<u8> m_ShaderBlob{};
	ShaderStage m_ShaderStage = ShaderStage::kUnknown;
	const char* m_pEntryPoint = "main";
};


BV_CREATE_CAST_TO_VK(IBvShader)
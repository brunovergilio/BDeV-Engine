#pragma once


#include "BDeV/Core/RenderAPI/BvShader.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


BV_OBJECT_DEFINE_ID(BvShaderVk, "73dd3e67-1619-4f36-a29e-99bf0f90ea0b");
class BvShaderVk final : public BvShader
{
	BV_NOCOPYMOVE(BvShaderVk);

public:
	BvShaderVk(BvRenderDeviceVk* pDevice, const ShaderCreateDesc& shaderCreateDesc);
	~BvShaderVk();
	BvRenderDevice* GetDevice() override;

	BV_INLINE bool IsValid() const { return m_ShaderBlob.Size() > 0; }

	BV_OBJECT_IMPL_INTERFACE(BvShaderVk, BvShader, IBvRenderDeviceObject);

private:
	void Compile(const ShaderCreateDesc& shaderCreateDesc);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvShaderVk);


BV_CREATE_CAST_TO_VK(BvShader)
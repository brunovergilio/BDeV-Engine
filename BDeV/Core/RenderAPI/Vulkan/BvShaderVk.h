#pragma once


#include "BDeV/Core/RenderAPI/BvShader.h"
#include "BvCommonVk.h"


class BvRenderDeviceVk;


class BvShaderVk final : public BvShader
{
	BV_NOCOPYMOVE(BvShaderVk);

public:
	BvShaderVk(BvRenderDeviceVk* pDevice, const ShaderCreateDesc& shaderCreateDesc);
	~BvShaderVk();
	BvRenderDevice* GetDevice() override;

private:
	void Compile(const ShaderCreateDesc& shaderCreateDesc);

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
};


BV_CREATE_CAST_TO_VK(BvShader)
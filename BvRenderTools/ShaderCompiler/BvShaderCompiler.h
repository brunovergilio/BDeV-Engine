#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvString.h"
#include "BDeV/RenderAPI/BvShaderResource.h"
#include "BDeV/RenderAPI/BvPipelineState.h"
#include "BDeV/Utils/BvUtils.h"


enum class ShaderLanguage : u8
{
	kVKSL, // Same as kGLSL, but for Vulkan
	kGLSL,
	kHLSL,
};


//Vulkan 1.0 supports SPIR - V 1.0
//Vulkan 1.1 supports SPIR - V 1.3 and below
//Vulkan 1.2 supports SPIR - V 1.5 and below
//Vulkan 1.3 supports SPIR - V 1.6 and below
enum class ShaderLanguageTarget : u8
{
	kSPIRV_1_0,
	kSPIRV_1_1,
	kSPIRV_1_2,
	kSPIRV_1_3,
	kSPIRV_1_4,
	kSPIRV_1_5,
	kSPIRV_1_6,
};


enum class ShaderAPIVersion : u8
{
	kVulkan_1_0,
	kVulkan_1_1,
	kVulkan_1_2,
	kVulkan_1_3,
	kDirectX_12,
	kOpenGL_4_5,
};


struct ShaderDesc
{
	BvString entryPoint = "";
	ShaderStage shaderStage = ShaderStage::kUnknown;
	ShaderLanguage shaderLanguage = ShaderLanguage::kVKSL;
	ShaderAPIVersion apiVersion = ShaderAPIVersion::kVulkan_1_0;
	ShaderLanguageTarget shaderTarget = ShaderLanguageTarget::kSPIRV_1_0;
};


class IBvShaderBlob
{
	BV_NOCOPYMOVE(IBvShaderBlob);

public:
	virtual const u8* GetBufferPointer() const = 0;
	virtual u64 GetBufferSize() const = 0;

protected:
	IBvShaderBlob() {}
	~IBvShaderBlob() {}
};


class IBvShaderCompiler
{
	BV_NOCOPYMOVE(IBvShaderCompiler);

public:
	virtual bool CompileFromFile(const char * const pFilename, const ShaderDesc& shaderDesc, IBvShaderBlob*& pShaderBlob, BvString* pErrorString = nullptr) = 0;
	virtual bool CompileFromMemory(const u8* const pBlob, u32 blobSize, const ShaderDesc& shaderDesc, IBvShaderBlob*& pShaderBlob, BvString* pErrorString = nullptr) = 0;

	virtual void DestroyShader(IBvShaderBlob*& pCompiledShader) = 0;

protected:
	IBvShaderCompiler() {}
	virtual ~IBvShaderCompiler() {}
};


namespace BvRenderTools
{
	extern "C"
	{
		BV_API IBvShaderCompiler* GetShaderCompiler();
	}
}
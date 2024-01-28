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
	virtual const BvVector<u8>& GetShaderBlob() const = 0;
	virtual const BvString& GetErrorString() const = 0;
	virtual bool IsValid() const = 0;

protected:
	IBvShaderBlob() {}
	~IBvShaderBlob() {}
};


class IBvShaderReflectionData
{
	BV_NOCOPYMOVE(IBvShaderReflectionData);

public:
	virtual const ShaderResourceLayoutDesc& GetResourceLayout() const = 0;
	virtual const BvVector<VertexInputDesc>& GetVertexAttributes() const = 0;

protected:
	IBvShaderReflectionData() {}
	virtual ~IBvShaderReflectionData() {}
};


class IBvShaderCompiler
{
	BV_NOCOPYMOVE(IBvShaderCompiler);

public:
	virtual IBvShaderBlob* CompileFromFile(const char * const pFilename, const ShaderDesc& shaderDesc) = 0;
	virtual IBvShaderBlob* CompileFromMemory(const u8* const pBlob, u32 blobSize, const ShaderDesc& shaderDesc) = 0;

	virtual void DestroyShader(IBvShaderBlob*& pCompiledShader) = 0;
	virtual void DestroyAllShaders() = 0;

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
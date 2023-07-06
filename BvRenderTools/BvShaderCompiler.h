#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvString.h"
#include "BDeV/RenderAPI/BvShaderResource.h"
#include "BDeV/RenderAPI/BvPipelineState.h"
#include "BDeV/Utils/BvUtils.h"


enum class ShaderLanguage : u8
{
	kGLSL,
	kHLSL,
};


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
};


struct ShaderDesc
{
	BvString entryPoint = "";
	ShaderStage shaderStage = ShaderStage::kUnknown;
	ShaderLanguage shaderLanguage = ShaderLanguage::kGLSL;
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
	virtual IBvShaderReflectionData* ReflectShader(const u8* pBlob, const u32 blobSize, ShaderLanguage shaderLanguage) = 0;
	IBvShaderReflectionData* ReflectShader(IBvShaderBlob* pBlob, ShaderLanguage shaderLanguage)
	{
		return ReflectShader(pBlob->GetShaderBlob().Data(), (u32)pBlob->GetShaderBlob().Size(), shaderLanguage);
	}

	virtual void DestroyShader(IBvShaderBlob*& pCompiledShader) = 0;
	virtual void DestroyAllShaders() = 0;

protected:
	IBvShaderCompiler() {}
	virtual ~IBvShaderCompiler() {}
};


namespace BvRenderTools
{
#if defined(BV_PLUGIN_DLL_EXPORT)
	BV_EXTERN_C
#endif
	BV_PLUGIN_API IBvShaderCompiler* GetShaderCompiler();
}
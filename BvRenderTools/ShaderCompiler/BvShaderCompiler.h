#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Container/BvString.h"
#include "BDeV/Core/RenderAPI/BvShaderResource.h"
#include "BDeV/Core/RenderAPI/BvPipelineState.h"
#include "BDeV/Core/Utils/BvUtils.h"


struct ShaderDesc
{
	const char* pEntryPoint = "main";
	ShaderStage shaderStage = ShaderStage::kUnknown;
	ShaderLanguage shaderLanguage = ShaderLanguage::kUnknown;
	ShaderCompiler compiler = ShaderCompiler::kUnknown;
	ShaderTarget shaderTarget = ShaderTarget::kUnknown;
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
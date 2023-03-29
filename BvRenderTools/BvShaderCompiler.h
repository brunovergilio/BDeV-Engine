#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Container/BvString.h"
#include "BDeV/Render/BvShaderResource.h"
#include "BDeV/Utils/BvUtils.h"


enum class ShaderLanguage : u8
{
	kGLSL,
};


struct ShaderDesc
{
	BvString entryPoint = "";
	ShaderStage shaderStage = ShaderStage::kUnknown;
	ShaderLanguage shaderLanguage = ShaderLanguage::kGLSL;
	// Version
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


class IBvShaderCompiler
{
	BV_NOCOPYMOVE(IBvShaderCompiler);

public:
	virtual IBvShaderBlob* CompileFromFile(const char * const pFilename, const ShaderDesc& shaderDesc) = 0;
	virtual IBvShaderBlob* CompileFromMemory(const u8* const pBlob, const size_t blobSize, const ShaderDesc& shaderDesc) = 0;
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
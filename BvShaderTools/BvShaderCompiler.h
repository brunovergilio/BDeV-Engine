#pragma once


#include "BvCore/BvCore.h"
#include "BvCore/Container/BvString.h"
#include "BvRender/BvShaderResource.h"
#include "BvCore/Utils/BvUtils.h"


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
	virtual const u8* const GetBufferPointer() const = 0;
	virtual const size_t GetBufferSize() const = 0;

protected:
	IBvShaderBlob() {}
	~IBvShaderBlob() {}
};


class IBvShaderCompiler
{
	BV_NOCOPYMOVE(IBvShaderCompiler);

public:
	virtual IBvShaderBlob* CompileFromFile(const char * const pFilename, const ShaderDesc& shaderDesc,
		BvString* const pErrors = nullptr) const = 0;
	virtual IBvShaderBlob* CompileFromMemory(const u8* const pBlob, const size_t blobSize, const ShaderDesc& shaderDesc,
		BvString* const pErrors = nullptr) const = 0;
	virtual void DestroyShader(IBvShaderBlob*& pCompiledShader) const = 0;

protected:
	IBvShaderCompiler() {}
	virtual ~IBvShaderCompiler() {}
};


IBvShaderCompiler* CreateShaderCompiler();
void DestroyShaderCompiler(IBvShaderCompiler*& pCompiler);
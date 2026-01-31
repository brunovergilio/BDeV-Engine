#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"


class IBvShaderBlob : public BvRCObj
{
	BV_NOCOPYMOVE(IBvShaderBlob);

public:
	virtual const void* GetBufferPointer() const = 0;
	virtual u64 GetBufferSize() const = 0;

protected:
	IBvShaderBlob() {}
	~IBvShaderBlob() {}
};
BV_OBJECT_DEFINE_ID(IBvShaderBlob, "36ffa34a-8f22-4cbc-b6c1-e6ec38c1d6dc");


class IBvShaderCompiler : public BvRCObj
{
	BV_NOCOPYMOVE(IBvShaderCompiler);

public:
	template<BvRCType T>
	BV_INLINE bool Compile(const ShaderSourceDesc& shaderDesc, T** ppShaderBlob, T** ppErrorBlob = nullptr)
	{
		return CompileImpl(shaderDesc, reinterpret_cast<void**>(ppShaderBlob), reinterpret_cast<void**>(ppErrorBlob));
	}

	template<BvRCType T>
	BV_INLINE bool CompileFromFile(const char* pFilename, const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob = nullptr)
	{
		return CompileFromFileImpl(pFilename, shaderDesc, reinterpret_cast<void**>(ppShaderBlob), reinterpret_cast<void**>(ppErrorBlob));
	}

protected:
	IBvShaderCompiler() {}
	~IBvShaderCompiler() {}

	virtual bool CompileImpl(const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob = nullptr) = 0;
	virtual bool CompileFromFileImpl(const char* pFilename, const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob = nullptr) = 0;
};
BV_OBJECT_DEFINE_ID(IBvShaderCompiler, "c6abe1b0-0c08-4da9-9faa-d6585d39ea8f");
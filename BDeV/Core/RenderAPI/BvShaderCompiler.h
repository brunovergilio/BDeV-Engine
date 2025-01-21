#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/Utils/BvObject.h"


class IBvShaderBlob
{
	BV_NOCOPYMOVE(IBvShaderBlob);

public:
	virtual const void* GetBufferPointer() const = 0;
	virtual u64 GetBufferSize() const = 0;

protected:
	IBvShaderBlob() {}
	virtual ~IBvShaderBlob() {}
};


class IBvShaderCompiler
{
	BV_NOCOPYMOVE(IBvShaderCompiler);

public:
	virtual bool Compile(const ShaderCreateDesc& shaderDesc, IBvShaderBlob** ppShaderBlob, IBvShaderBlob** ppErrorBlob = nullptr) = 0;

protected:
	IBvShaderCompiler() {}
	virtual ~IBvShaderCompiler() {}
};
#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/Utils/BvObject.h"


class IBvShaderBlob : public IBvObject
{
	BV_NOCOPYMOVE(IBvShaderBlob);

public:
	BV_IBVOBJECT_IMPL_INTERFACE_BASE(IBvShaderBlob);

	virtual const void* GetBufferPointer() const = 0;
	virtual u64 GetBufferSize() const = 0;

protected:
	IBvShaderBlob() {}
	virtual ~IBvShaderBlob() {}
};


class IBvShaderCompiler : public IBvObject
{
	BV_NOCOPYMOVE(IBvShaderCompiler);

public:
	BV_IBVOBJECT_IMPL_INTERFACE_BASE(IBvShaderCompiler);

	virtual bool Compile(const ShaderCreateDesc& shaderDesc, IBvShaderBlob** ppShaderBlob, IBvShaderBlob** ppErrorBlob = nullptr) = 0;

protected:
	IBvShaderCompiler() {}
	virtual ~IBvShaderCompiler() {}
};
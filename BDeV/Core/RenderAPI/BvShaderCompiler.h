#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/Utils/BvObject.h"


BV_OBJECT_DEFINE_ID(IBvShaderBlob, "36ffa34a-8f22-4cbc-b6c1-e6ec38c1d6dc");
class IBvShaderBlob : public BvObjectBase
{
	BV_NOCOPYMOVE(IBvShaderBlob);

public:
	virtual const void* GetBufferPointer() const = 0;
	virtual u64 GetBufferSize() const = 0;

protected:
	IBvShaderBlob() {}
	~IBvShaderBlob() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvShaderBlob);


BV_OBJECT_DEFINE_ID(IBvShaderCompiler, "c6abe1b0-0c08-4da9-9faa-d6585d39ea8f");
class IBvShaderCompiler : public BvObjectBase
{
	BV_NOCOPYMOVE(IBvShaderCompiler);

public:
	virtual bool Compile(const ShaderCreateDesc& shaderDesc, IBvShaderBlob** ppShaderBlob, IBvShaderBlob** ppErrorBlob = nullptr) = 0;
	virtual bool CompileFromFile(const char* pFilename, const ShaderCreateDesc& shaderDesc, IBvShaderBlob** ppShaderBlob, IBvShaderBlob** ppErrorBlob = nullptr) = 0;

protected:
	IBvShaderCompiler() {}
	~IBvShaderCompiler() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvShaderCompiler);
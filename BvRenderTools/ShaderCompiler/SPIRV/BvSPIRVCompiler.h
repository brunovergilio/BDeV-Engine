#pragma once


#include "BDeV/Core/Container/BvRobinSet.h"
#include "BDeV/Core/System/File/BvPath.h"
#include "BvRenderTools/ShaderCompiler/BvShaderBlob.h"


BV_IBVOBJECT_CREATE_REFCOUNT_BASE(BvShaderCompilerRefCount, IBvShaderCompiler);


class BvSPIRVCompiler final : public BvShaderCompilerRefCount
{
	BV_NOCOPYMOVE(BvSPIRVCompiler);

public:
	BV_IBVOBJECT_IMPL_INTERFACE(BvSPIRVCompiler, IBvShaderCompiler);

	BvSPIRVCompiler();
	~BvSPIRVCompiler();

	bool Compile(const ShaderCreateDesc& shaderDesc, IBvShaderBlob** ppShaderBlob, IBvShaderBlob** ppErrorBlob = nullptr) override;

private:
	void ProcessIncludes(const char* pFirst, u32 size, BvString& result, const BvPath& basePath);
};


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateSPIRVCompiler(IBvShaderCompiler** ppCompiler);
	}
}
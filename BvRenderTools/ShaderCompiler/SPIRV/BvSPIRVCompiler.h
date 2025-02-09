#pragma once


#include "BDeV/Core/Container/BvRobinSet.h"
#include "BDeV/Core/System/File/BvPath.h"
#include "BvRenderTools/ShaderCompiler/BvShaderBlob.h"


class BvSPIRVCompiler final : public IBvShaderCompiler
{
	BV_NOCOPYMOVE(BvSPIRVCompiler);

public:
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
		BV_API IBvShaderCompiler* GetSPIRVCompiler();
	}
}
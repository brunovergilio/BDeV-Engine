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

private:
	bool CompileImpl(const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob = nullptr) override;
	bool CompileFromFileImpl(const char* pFilename, const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob = nullptr) override;

	void ProcessIncludes(const char* pFirst, u32 size, BvString& result, const BvPath& basePath);
};
BV_OBJECT_DEFINE_ID(BvSPIRVCompiler, "7816e1cf-31fe-4866-b832-e8800c57ecfa");


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateSPIRVCompiler(void** ppObj);
	}
}
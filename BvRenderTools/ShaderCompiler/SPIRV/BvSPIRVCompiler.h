#pragma once


#include "BDeV/Core/Container/BvRobinSet.h"
#include "BDeV/Core/System/File/BvPath.h"
#include "BvRenderTools/ShaderCompiler/BvShaderBlob.h"


BV_OBJECT_DEFINE_ID(BvSPIRVCompiler, "7816e1cf-31fe-4866-b832-e8800c57ecfa");
class BvSPIRVCompiler final : public IBvShaderCompiler
{
	BV_NOCOPYMOVE(BvSPIRVCompiler);

public:
	BvSPIRVCompiler();
	~BvSPIRVCompiler();

	bool Compile(const ShaderCreateDesc& shaderDesc, IBvShaderBlob** ppShaderBlob, IBvShaderBlob** ppErrorBlob = nullptr) override;
	bool CompileFromFile(const char* pFilename, const ShaderCreateDesc& shaderDesc, IBvShaderBlob** ppShaderBlob, IBvShaderBlob** ppErrorBlob = nullptr) override;

	BV_OBJECT_IMPL_INTERFACE(BvSPIRVCompiler, IBvShaderCompiler);

private:
	void ProcessIncludes(const char* pFirst, u32 size, BvString& result, const BvPath& basePath);
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvSPIRVCompiler);


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateSPIRVCompiler(IBvShaderCompiler** ppObj);
	}
}
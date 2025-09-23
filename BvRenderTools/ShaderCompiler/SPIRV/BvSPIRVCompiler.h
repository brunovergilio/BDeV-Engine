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

	//BV_OBJECT_IMPL_INTERFACE(BvSPIRVCompiler, IBvShaderCompiler);

private:
	bool Compile(const ShaderCreateDesc& shaderDesc, const BvUUID& objId, void** ppShaderBlob, void** ppErrorBlob = nullptr) override;
	bool CompileFromFile(const char* pFilename, const ShaderCreateDesc& shaderDesc, const BvUUID& objId, void** ppShaderBlob, void** ppErrorBlob = nullptr) override;

	void ProcessIncludes(const char* pFirst, u32 size, BvString& result, const BvPath& basePath);
	void SelfDestroy() override;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvSPIRVCompiler);


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateSPIRVCompiler(const BvUUID& objId, void** ppObj);
	}
}
#pragma once


#include "BDeV/Core/Container/BvRobinSet.h"
#include "BvRenderTools/ShaderCompiler/BvShaderBlob.h"
#include "BDeV/Core/System/Library/BvSharedLib.h"
#include <wrl/client.h>
#include "Third Party/dxcapi.h"


using Microsoft::WRL::ComPtr;


class BvDXCompiler final : public IBvShaderCompiler
{
	BV_NOCOPYMOVE(BvDXCompiler);

public:
	BvDXCompiler();
	~BvDXCompiler();

private:
	bool CompileImpl(const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob = nullptr) override;
	bool CompileFromFileImpl(const char* pFilename, const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob = nullptr) override;

private:
	BvSharedLib m_DXCLib;
	ComPtr<IDxcUtils> m_Utils;
	ComPtr<IDxcCompiler3> m_Compiler;
};


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateDXCompiler(void** ppObj);
	}
}
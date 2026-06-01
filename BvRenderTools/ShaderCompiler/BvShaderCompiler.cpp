#include "BvShaderCompiler.h"








bool CompileWithGlslang(const ShaderSourceDesc& shaderDesc, BvVector<u8>& spv, BvVector<u8>& errors)
{
	return false;
}


bool CompileWithDXC(const ShaderSourceDesc& shaderDesc, BvVector<u8>& spv, BvVector<u8>& errors)
{
	return false;
}


bool ConvertSPIRVToHLSL(const BvVector<u8>& spv, BvVector<u8>& hlsl)
{
	return false;
}


BvShaderCompiler::BvShaderCompiler()
{
}


BvShaderCompiler::~BvShaderCompiler()
{
}


bool BvShaderCompiler::CompileImpl(const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob /*= nullptr*/)
{
	return false;
}


bool BvShaderCompiler::CompileFromFileImpl(const char* pFilename, const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob /*= nullptr*/)
{
	BvFile file(pFilename, BvFileAccessMode::kRead, BvFileAction::kOpen);
	if (!file.IsValid())
	{
		if (ppErrorBlob)
		{
			*ppErrorBlob = BV_RC_CREATE(BvShaderBlob, "Error opening the file.");
		}
		return false;
	}
	auto size = file.GetSize();
	BvVector<char> sourceCode(size);
	file.Read(sourceCode.Data(), size);

	auto& desc = const_cast<ShaderSourceDesc&>(shaderDesc);
	desc.m_pSourceCode = sourceCode.Data();
	desc.m_SourceCodeSize = size;

	return CompileImpl(shaderDesc, ppShaderBlob, ppErrorBlob);
}
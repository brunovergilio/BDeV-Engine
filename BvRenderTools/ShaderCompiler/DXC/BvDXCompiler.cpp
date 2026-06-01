#include "BvDXCompiler.h"
#include "BvDXCUtils.h"
#include "BvRenderTools/ShaderCompiler/BvShaderBlob.h"
#include "BDeV/Core/System/File/BvFile.h"


BvDXCompiler::BvDXCompiler()
	: m_DXCLib("dxcompiler.dll")
{
	if (!m_DXCLib)
	{
		return;
	}

	DxcCreateInstanceProc pDXCCreateInstanceFn = m_DXCLib.GetProcAddressT<DxcCreateInstanceProc>("DxcCreateInstance");
	auto hr = pDXCCreateInstanceFn(CLSID_DxcUtils, IID_PPV_ARGS(&m_Utils));
	if (FAILED(hr))
	{
		return;
	}

	hr = pDXCCreateInstanceFn(CLSID_DxcCompiler, IID_PPV_ARGS(&m_Compiler));
	if (FAILED(hr))
	{
		m_Utils.Reset();
	}
}


BvDXCompiler::~BvDXCompiler()
{
	m_Compiler.Reset();
	m_Utils.Reset();
	m_DXCLib.Close();
}


bool BvDXCompiler::CompileImpl(const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob /*= nullptr*/)
{
	BvVector<u8> compiledShaderBlob;
	BvVector<u8> errorMessage;
	if (!BvDXCUtils::CompileToDXIL(m_Utils.Get(), m_Compiler.Get(), shaderDesc, compiledShaderBlob, errorMessage))
	{
		if (ppErrorBlob)
		{
			*ppErrorBlob = BV_RC_CREATE(BvShaderBlob, errorMessage);
		}

		return false;
	}

	*ppShaderBlob = BV_RC_CREATE(BvShaderBlob, compiledShaderBlob);

	return true;
}


bool BvDXCompiler::CompileFromFileImpl(const char* pFilename, const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob /*= nullptr*/)
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


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateDXCompiler(void** ppObj)
		{
			*ppObj = BV_RC_CREATE(BvDXCompiler);
			return true;
		}
	}
}
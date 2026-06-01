#include "BvDXCUtils.h"
#include <wrl/client.h>


using Microsoft::WRL::ComPtr;


namespace BvDXCUtils
{
	const wchar_t* GetShaderStage(ShaderStage shaderStage)
	{
		switch (shaderStage)
		{
		case ShaderStage::kVertex: return L"vs";
		case ShaderStage::kHullOrControl: return L"hs";
		case ShaderStage::kDomainOrEvaluation: return L"ds";
		case ShaderStage::kGeometry: return L"gs";
		case ShaderStage::kPixelOrFragment: return L"ps";
		case ShaderStage::kCompute: return L"cs";
		case ShaderStage::kMesh: return L"ms";
		case ShaderStage::kAmplificationOrTask: return L"as";
		case ShaderStage::kRayGen:
		case ShaderStage::kAnyHit:
		case ShaderStage::kClosestHit:
		case ShaderStage::kMiss:
		case ShaderStage::kIntersection:
		case ShaderStage::kCallable:
			return L"lib";
		}

		return L"";
	}


	const wchar_t* GetTargetProfile(ShaderTarget shaderTarget)
	{
		switch (shaderTarget)
		{
		case ShaderTarget::kHLSL_5_0: return L"_5_0";
		case ShaderTarget::kHLSL_5_1: return L"_5_1";
		case ShaderTarget::kHLSL_6_0: return L"_6_0";
		case ShaderTarget::kHLSL_6_1: return L"_6_1";
		case ShaderTarget::kHLSL_6_2: return L"_6_2";
		case ShaderTarget::kHLSL_6_3: return L"_6_3";
		case ShaderTarget::kHLSL_6_4: return L"_6_4";
		case ShaderTarget::kHLSL_6_5: return L"_6_5";
		case ShaderTarget::kHLSL_6_6: return L"_6_6";
		case ShaderTarget::kHLSL_6_7: return L"_6_7";
		case ShaderTarget::kHLSL_6_8: return L"_6_8";
		case ShaderTarget::kHLSL_6_9: return L"_6_9";
		}

		return L"_6_5";
	}


	const wchar_t* GetSPIRVTargetProfile(ShaderTarget shaderTarget)
	{
		switch (shaderTarget)
		{
		case ShaderTarget::kSPIRV_1_0: return L"vulkan1.0";
		case ShaderTarget::kSPIRV_1_1: return L"vulkan1.1";
		case ShaderTarget::kSPIRV_1_2: return L"vulkan1.1";
		case ShaderTarget::kSPIRV_1_3: return L"vulkan1.1";
		case ShaderTarget::kSPIRV_1_4: return L"vulkan1.1spirv1.4";
		case ShaderTarget::kSPIRV_1_5: return L"vulkan1.2";
		case ShaderTarget::kSPIRV_1_6: return L"vulkan1.3";
		}

		return nullptr;
	}


	bool CompileToDXIL(IDxcUtils* pUtils, IDxcCompiler3* pCompiler, const ShaderSourceDesc& shaderDesc, BvVector<u8>& result, BvVector<u8>& errors)
	{
		ComPtr<IDxcBlobEncoding> source;
		auto hr = pUtils->CreateBlob(shaderDesc.m_pSourceCode, shaderDesc.m_SourceCodeSize, CP_UTF8, &source);
		if (FAILED(hr))
		{
			return false;
		}

		wchar_t entryPoint[64];
		size_t size = mbstowcs(nullptr, shaderDesc.m_pEntryPoint, 0) + 1;
		mbstowcs(entryPoint, shaderDesc.m_pEntryPoint, size);

		wchar_t profile[64]{};
		wcscat(profile, GetShaderStage(shaderDesc.m_ShaderStage));
		wcscat(profile, GetTargetProfile(shaderDesc.m_ShaderTarget));
		if (auto pSpv = GetSPIRVTargetProfile(shaderDesc.m_ShaderTarget))
		{
			wcscat(profile, L" -spirv -fspv-target-env=");
			wcscat(profile, pSpv);
		}

		BvVector<LPCWSTR> arguments;
		// -E for the entry point (eg. 'main')
		arguments.EmplaceBack(L"-E");
		arguments.EmplaceBack(entryPoint);

		// -T for the target profile (eg. 'ps_6_6')
		arguments.EmplaceBack(L"-T");
		arguments.EmplaceBack(profile);

		// Strip reflection data and pdbs
		arguments.EmplaceBack(L"-Qstrip_debug");
		arguments.EmplaceBack(L"-Qstrip_reflect");

		arguments.EmplaceBack(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
		arguments.EmplaceBack(DXC_ARG_DEBUG); //-Zi

		//for (const std::wstring& define : defines)
		//{
		//	arguments.EmplaceBack(L"-D");
		//	arguments.EmplaceBack(define.c_str());
		//}

		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = source->GetBufferPointer();
		sourceBuffer.Size = source->GetBufferSize();
		sourceBuffer.Encoding = 0;

		ComPtr<IDxcResult> compileResult;
		hr = pCompiler->Compile(&sourceBuffer, arguments.Data(), (u32)arguments.Size(), nullptr, IID_PPV_ARGS(&compileResult));
		if (FAILED(hr))
		{
			return false;
		}

		// Error Handling. Note that this will also include warnings unless disabled.
		ComPtr<IDxcBlobUtf8> errorBlob;
		hr = compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorBlob), nullptr);
		if (FAILED(hr))
		{
			return false;
		}
		else if (errorBlob && errorBlob->GetStringLength() > 0)
		{
			errors.Resize(errorBlob->GetStringLength());
			memcpy(&errors[0], errorBlob->GetStringPointer(), errorBlob->GetStringLength());

			return false;
		}

		// DXC parts
		//DXC_OUT_OBJECT
		//DXC_OUT_ERRORS
		//DXC_OUT_PDB
		//DXC_OUT_SHADER_HASH
		//DXC_OUT_DISASSEMBLY
		//DXC_OUT_HLSL
		//DXC_OUT_TEXT
		//DXC_OUT_REFLECTION
		//DXC_OUT_ROOT_SIGNATURE

		ComPtr<IDxcBlob> compiledBlob;
		hr = compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiledBlob), nullptr);
		if (FAILED(hr))
		{
			return false;
		}

		if (compiledBlob && compiledBlob->GetBufferSize() > 0)
		{
			result.Resize(compiledBlob->GetBufferSize());
			memcpy(&result[0], compiledBlob->GetBufferPointer(), compiledBlob->GetBufferSize());
		}

		return result.Size() != 0;
	}
}
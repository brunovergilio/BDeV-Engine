#include "BvSPIRVCompiler.h"
#include "BvRenderTools/ShaderCompiler/BvShaderBlob.h"

#if (BV_PLATFORM == BV_PLATFORM_WIN32 && BV_COMPILER == BV_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable:26451 26495)
#endif


#include "BvSPIRVCompiler.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "spirv-tools/optimizer.hpp"
#include "glslang/Public/ResourceLimits.h"


constexpr TBuiltInResource g_DefaultTBuiltInResource =
{
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,
	/* .maxMeshOutputVerticesNV = */ 256,
	/* .maxMeshOutputPrimitivesNV = */ 512,
	/* .maxMeshWorkGroupSizeX_NV = */ 32,
	/* .maxMeshWorkGroupSizeY_NV = */ 1,
	/* .maxMeshWorkGroupSizeZ_NV = */ 1,
	/* .maxTaskWorkGroupSizeX_NV = */ 32,
	/* .maxTaskWorkGroupSizeY_NV = */ 1,
	/* .maxTaskWorkGroupSizeZ_NV = */ 1,
	/* .maxMeshViewCountNV = */ 4,
	/* .maxMeshOutputVerticesEXT = */ 256,
	/* .maxMeshOutputPrimitivesEXT = */ 256,
	/* .maxMeshWorkGroupSizeX_EXT = */ 128,
	/* .maxMeshWorkGroupSizeY_EXT = */ 128,
	/* .maxMeshWorkGroupSizeZ_EXT = */ 128,
	/* .maxTaskWorkGroupSizeX_EXT = */ 128,
	/* .maxTaskWorkGroupSizeY_EXT = */ 128,
	/* .maxTaskWorkGroupSizeZ_EXT = */ 128,
	/* .maxMeshViewCountEXT = */ 4,
	/* .maxDualSourceDrawBuffersEXT = */ 1,

	/* .limits = */
	{
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1,
	}
};


class GlslangControl
{
public:
	GlslangControl()
	{
		glslang::InitializeProcess();
	}

	~GlslangControl()
	{
		glslang::FinalizeProcess();
	}
};


EShLanguage GetShaderStage(const ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::kVertex:				return EShLanguage::EShLangVertex;
	case ShaderStage::kHullOrControl:		return EShLanguage::EShLangTessControl;
	case ShaderStage::kDomainOrEvaluation:	return EShLanguage::EShLangTessEvaluation;
	case ShaderStage::kGeometry:			return EShLanguage::EShLangGeometry;
	case ShaderStage::kPixelOrFragment:		return EShLanguage::EShLangFragment;
	case ShaderStage::kCompute:				return EShLanguage::EShLangCompute;
	case ShaderStage::kMesh:				return EShLanguage::EShLangMesh;
	case ShaderStage::kAmplificationOrTask:	return EShLanguage::EShLangTask;
	default:
		BV_ASSERT(0, "This code should be unreachable");
		return EShLanguage::EShLangCount;
	}
}


glslang::EShTargetClientVersion GetAPIVersion(ShaderTarget target)
{
	return target != ShaderTarget::kOpenGL_4_5 ? glslang::EShTargetClientVersion::EShTargetVulkan_1_3 : glslang::EShTargetClientVersion::EShTargetOpenGL_450;
}


glslang::EShTargetLanguageVersion GetShaderTarget(ShaderTarget target)
{
	switch (target)
	{
	case ShaderTarget::kOpenGL_4_5:
	case ShaderTarget::kSPIRV_1_0: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_0;
	case ShaderTarget::kSPIRV_1_1: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_1;
	case ShaderTarget::kSPIRV_1_2: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_2;
	case ShaderTarget::kSPIRV_1_3: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_3;
	case ShaderTarget::kSPIRV_1_4: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_4;
	case ShaderTarget::kSPIRV_1_5: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_5;
	case ShaderTarget::kUnknown:
	case ShaderTarget::kSPIRV_1_6: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_6;
	default:
		BV_ASSERT(0, "This code should be unreachable");
		return glslang::EShTargetLanguageVersion::EShTargetSpv_1_0;
	}
}


spv_target_env GetTargetEnv(ShaderTarget target)
{
	switch (target)
	{
	case ShaderTarget::kSPIRV_1_0: return spv_target_env::SPV_ENV_UNIVERSAL_1_0;
	case ShaderTarget::kSPIRV_1_1: return spv_target_env::SPV_ENV_UNIVERSAL_1_1;
	case ShaderTarget::kSPIRV_1_2: return spv_target_env::SPV_ENV_UNIVERSAL_1_2;
	case ShaderTarget::kSPIRV_1_3: return spv_target_env::SPV_ENV_UNIVERSAL_1_3;
	case ShaderTarget::kSPIRV_1_4: return spv_target_env::SPV_ENV_UNIVERSAL_1_4;
	case ShaderTarget::kSPIRV_1_5: return spv_target_env::SPV_ENV_UNIVERSAL_1_5;
	case ShaderTarget::kUnknown:
	case ShaderTarget::kSPIRV_1_6: return spv_target_env::SPV_ENV_UNIVERSAL_1_6;
	case ShaderTarget::kOpenGL_4_5: return spv_target_env::SPV_ENV_OPENGL_4_5;
	default:
		BV_ASSERT(0, "This code should be unreachable");
		return spv_target_env::SPV_ENV_UNIVERSAL_1_0;
	}
}


int GetAPIVersionValue(ShaderTarget target)
{
	return target != ShaderTarget::kOpenGL_4_5 ? 130 : 100;
}


BvSPIRVCompiler::BvSPIRVCompiler()
{
	static GlslangControl control;
}


BvSPIRVCompiler::~BvSPIRVCompiler()
{
}


bool BvSPIRVCompiler::Compile(const ShaderCreateDesc& shaderDesc, IBvShaderBlob** ppShaderBlob, IBvShaderBlob** ppErrorBlob)
{
	auto shaderStage = GetShaderStage(shaderDesc.m_ShaderStage);
	auto targetClient = GetAPIVersion(shaderDesc.m_ShaderTarget);
	auto targetLanguage = GetShaderTarget(shaderDesc.m_ShaderTarget);
	auto version = GetAPIVersionValue(shaderDesc.m_ShaderTarget);
	auto language = shaderDesc.m_ShaderLanguage != ShaderLanguage::kHLSL ? glslang::EShSource::EShSourceGlsl : glslang::EShSource::EShSourceHlsl;
	auto client = shaderDesc.m_ShaderTarget != ShaderTarget::kOpenGL_4_5 ? glslang::EShClient::EShClientVulkan : glslang::EShClient::EShClientOpenGL;
	glslang::TShader shader(shaderStage);
	shader.setEnvInput(language, shaderStage, client, version);
	shader.setEnvClient(client, targetClient);
	shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, targetLanguage);
	shader.setEntryPoint(shaderDesc.m_pEntryPoint);

	const char* shaderStrings[] = { shaderDesc.m_pSourceCode };
	i32 lenghts[] = { static_cast<i32>(shaderDesc.m_SourceCodeSize) };
	shader.setStringsWithLengths(shaderStrings, lenghts, 1);
	//shader.setAutoMapBindings(true);

	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
	if (!shader.parse(&g_DefaultTBuiltInResource, version, false, messages))
	{
		if (ppErrorBlob)
		{
			*ppErrorBlob = BV_OBJECT_CREATE(BvShaderBlob, shader.getInfoLog());
		}
		return false;
	}

	glslang::TProgram program;
	program.addShader(&shader);
	if (!program.link(messages) || !program.mapIO())
	{
		if (ppErrorBlob)
		{
			*ppErrorBlob = BV_OBJECT_CREATE(BvShaderBlob, program.getInfoLog());
		}
		return false;
	}

	std::vector<u32> spv;
	glslang::GlslangToSpv(*program.getIntermediate(shader.getStage()), spv);

	std::vector<u32> spvOpt;
	spvtools::Optimizer optimizer(GetTargetEnv(shaderDesc.m_ShaderTarget));
	optimizer.RegisterPerformancePasses();
	if (!optimizer.Run(spv.data(), spv.size(), &spvOpt))
	{
		if (ppErrorBlob)
		{
			*ppErrorBlob = BV_OBJECT_CREATE(BvShaderBlob, "Error optimizing spirv binary.");
		}
		return false;
	}

	BvVector<u8> compiledShaderBlob(spvOpt.size() * sizeof(u32));
	memcpy(compiledShaderBlob.Data(), spvOpt.data(), compiledShaderBlob.Size());

	*ppShaderBlob = BV_OBJECT_CREATE(BvShaderBlob, compiledShaderBlob);

	return true;
}


bool BvSPIRVCompiler::CompileFromFile(const char* pFilename, const ShaderCreateDesc& shaderDesc, IBvShaderBlob** ppShaderBlob, IBvShaderBlob** ppErrorBlob)
{
	BvFile file(pFilename, BvFileAccessMode::kRead, BvFileAction::kOpen);
	if (!file.IsValid())
	{
		*ppErrorBlob = BV_OBJECT_CREATE(BvShaderBlob, "Error opening the file.");
		return false;
	}
	auto size = file.GetSize();
	BvVector<char> sourceCode(size);
	file.Read(sourceCode.Data(), size);

	auto& desc = const_cast<ShaderCreateDesc&>(shaderDesc);
	desc.m_pSourceCode = sourceCode.Data();
	desc.m_SourceCodeSize = size;

	return Compile(shaderDesc, ppShaderBlob, ppErrorBlob);
}


const char* SkipDelimitersAndComments(const char* pCurr, const char* pLast)
{
	while (pCurr != pLast && (std::isspace(*pCurr) || *pCurr == '/'))
	{
		if (*pCurr == '/')
		{
			++pCurr;
			if (*pCurr == '/')
			{
				++pCurr;
				while (pCurr != pLast && *pCurr != '\n')
				{
					++pCurr;
				}
			}
			else if (*pCurr == '*')
			{
				++pCurr;
				if (pCurr != pLast)
				{
					while (pCurr + 1 != pLast)
					{
						if (pCurr[0] == '*' && pCurr[1] == '/')
						{
							pCurr += 2;
							break;
						}
						++pCurr;
					}
				}
			}
		}
		else
		{
			++pCurr;
		}
	}

	return pCurr;
}


bool IsIncludeChar(char c)
{
	return c == '<' || c == '>' || c == '\'' || c == '\"';
}


void BvSPIRVCompiler::ProcessIncludes(const char* pFirst, u32 size, BvString& result, const BvPath& basePath)
{
	if (!basePath.IsValid())
	{
		return;
	}

	const char* pCurr = pFirst;
	const char* pLast = pFirst + size;
	const char* pLastWrite = pCurr;
	while (pCurr != pLast)
	{
		pCurr = SkipDelimitersAndComments(pCurr, pLast);
		if (*pCurr != '#')
		{
			if (pCurr != pLast)
			{
				++pCurr;
			}
			continue;
		}

		auto pHash = pCurr++;
		pCurr = SkipDelimitersAndComments(pCurr, pLast);
		auto pKeywordEnd = pCurr;
		while (pKeywordEnd != pLast && !std::isspace(*pKeywordEnd))
		{
			++pKeywordEnd;
		}

		if (u32(pKeywordEnd - pCurr) != 7 || strnicmp(pCurr, "include", 7) != 0)
		{
			pCurr = pKeywordEnd;
			continue;
		}

		// If we got here we have found '#include', so append all the code up to this point
		result.Append(pLastWrite, 0, u32(pHash - pLastWrite));

		pCurr = pKeywordEnd;
		pCurr = SkipDelimitersAndComments(pCurr, pLast);

		if (IsIncludeChar(*pCurr))
		{
			auto pPathFirst = ++pCurr;
			while (pCurr != pLast && !IsIncludeChar(*pCurr))
			{
				++pCurr;
			}

			if (pCurr != pLast)
			{
				auto pPathLast = pCurr;

				BvPath path(pPathFirst, u32(pPathLast - pPathFirst));
				if (!path.IsAbsolute())
				{
					path.PrependPath(basePath);
				}
				path.NormalizePath();

				auto file = path.AsFile(BvFileAccessMode::kRead);
				if (file.IsValid())
				{
					auto fileSize = file.GetSize();
					BvString blob((u32)fileSize);
					file.Read(&blob[0], fileSize);
					file.Close();

					path.MoveToParentPath();

					ProcessIncludes(blob.CStr(), blob.Size(), result, path);
				}
			}
		}

		// At this point we're either at the end or at the include character
		if (pCurr != pLast)
		{
			++pCurr;
		}

		pLastWrite = pCurr;
	}

	if (pLastWrite != pLast)
	{
		result.Append(pLastWrite, 0, u32(pLast - pLastWrite));
	}
}


namespace BvRenderTools
{
	extern "C"
	{
		BV_API bool CreateSPIRVCompiler(IBvShaderCompiler** ppObj)
		{
			*ppObj = BV_OBJECT_CREATE(BvSPIRVCompiler);
			return true;
		}
	}
}

#if (BV_PLATFORM == BV_PLATFORM_WIN32 && BV_COMPILER == BV_COMPILER_MSVC)
#pragma warning(pop)
#endif
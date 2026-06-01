#include "BvSPIRVUtils.h"


#if (BV_PLATFORM_WIN32 && BV_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable:26451 26495)
#endif


#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "spirv-tools/optimizer.hpp"
#include "spirv_cross/spirv_hlsl.hpp"


constexpr const TBuiltInResource g_DefaultTBuiltInResource =
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
	case ShaderStage::kRayGen:				return EShLanguage::EShLangRayGen;
	case ShaderStage::kIntersection:		return EShLanguage::EShLangIntersect;
	case ShaderStage::kAnyHit:				return EShLanguage::EShLangAnyHit;
	case ShaderStage::kClosestHit:			return EShLanguage::EShLangClosestHit;
	case ShaderStage::kMiss:				return EShLanguage::EShLangMiss;
	case ShaderStage::kCallable:			return EShLanguage::EShLangCallable;
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
	case ShaderTarget::kSPIRV_1_6:
	default: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_6;
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
		return spv_target_env::SPV_ENV_UNIVERSAL_1_6;
	}
}


int GetAPIVersionValue(ShaderTarget target)
{
	return target != ShaderTarget::kOpenGL_4_5 ? 130 : 100;
}


i32 GetSPVCTargetProfile(ShaderTarget shaderTarget)
{
	switch (shaderTarget)
	{
	case ShaderTarget::kHLSL_5_0: return 50;
	case ShaderTarget::kHLSL_5_1: return 51;
	case ShaderTarget::kHLSL_6_0: return 60;
	case ShaderTarget::kHLSL_6_1: return 61;
	case ShaderTarget::kHLSL_6_2: return 62;
	case ShaderTarget::kHLSL_6_3: return 63;
	case ShaderTarget::kHLSL_6_4: return 64;
	case ShaderTarget::kHLSL_6_5: return 65;
	case ShaderTarget::kHLSL_6_6: return 66;
	case ShaderTarget::kHLSL_6_7: return 67;
	case ShaderTarget::kHLSL_6_8: return 68;
	case ShaderTarget::kHLSL_6_9: return 69;
	}

	return 65;
}


namespace BvSPIRVUtils
{
	bool CompileSPIRV(const ShaderSourceDesc& shaderDesc, BvVector<u8>& result, BvVector<u8>& errorMsg)
	{
		static GlslangControl control;

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
			const auto pError = shader.getInfoLog();
			const auto errorSize = std::char_traits<char>::length(pError) + 1;
			errorMsg.Resize(errorSize);
			memcpy(errorMsg.Data(), pError, errorSize);

			return false;
		}

		glslang::TProgram program;
		program.addShader(&shader);
		if (!program.link(messages) || !program.mapIO())
		{
			const auto pError = shader.getInfoLog();
			const auto errorSize = std::char_traits<char>::length(pError) + 1;
			errorMsg.Resize(errorSize);
			memcpy(errorMsg.Data(), pError, errorSize);

			return false;
		}

		std::vector<u32> spv;
		glslang::GlslangToSpv(*program.getIntermediate(shader.getStage()), spv);

		std::vector<u32> spvOpt;
		spvtools::Optimizer optimizer(GetTargetEnv(shaderDesc.m_ShaderTarget));
		optimizer.RegisterPerformancePasses();


		if (!optimizer.Run(spv.data(), spv.size(), &spvOpt))
		{
			spvOpt = std::move(spv);
		}

		result.Resize(spvOpt.size() * sizeof(u32));
		memcpy(result.Data(), spvOpt.data(), result.Size());

		return true;
	}


	bool ConvertSPIRVToHLSL(const BvVector<u8>& spirvBytes, BvVector<u8>& result, ShaderTarget shaderTarget)
	{
		std::vector<u32> spirvWords(spirvBytes.Size() / sizeof(u32));
		memcpy(spirvWords.data(), spirvBytes.Data(), spirvBytes.Size());

		spirv_cross::CompilerHLSL compiler((const u32*)spirvBytes.Data(), spirvBytes.Size() / sizeof(u32));
		spirv_cross::CompilerHLSL::Options options;
		options.use_entry_point_name = true;
		options.shader_model = GetSPVCTargetProfile(shaderTarget);
		if (options.shader_model >= 62)
		{
			options.enable_16bit_types;
		}
		compiler.set_hlsl_options(options);

		auto hlsl = compiler.compile();
		if (hlsl.empty())
		{
			return false;
		}

		result.Resize(hlsl.size());
		memcpy(result.Data(), hlsl.data(), result.Size());

		return true;
	}
}


#if (BV_PLATFORM_WIN32 && BV_COMPILER_MSVC)
#pragma warning(pop)
#endif
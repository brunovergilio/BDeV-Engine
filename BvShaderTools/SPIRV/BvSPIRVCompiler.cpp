#include "BvSPIRVCompiler.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "spirv-tools/optimizer.hpp"


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
} g_GlslangControl;


EShLanguage GetEShShaderStage(const ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::kVertex:				return EShLanguage::EShLangVertex;
	case ShaderStage::kHullOrControl:		return EShLanguage::EShLangTessControl;
	case ShaderStage::kDomainOrEvaluation:	return EShLanguage::EShLangTessEvaluation;
	case ShaderStage::kGeometry:			return EShLanguage::EShLangGeometry;
	case ShaderStage::kPixelOrFragment:		return EShLanguage::EShLangFragment;
	case ShaderStage::kMesh:				return EShLanguage::EShLangMeshNV;
	case ShaderStage::kAmplificationOrTask:	return EShLanguage::EShLangTaskNV;
	case ShaderStage::kCompute:				return EShLanguage::EShLangCompute;
	default:
		BvAssert(0, "This code should be unreachable");
		return EShLanguage::EShLangCount;
	}
}


TBuiltInResource g_BuiltInResources = []()
{
	TBuiltInResource resources;
	resources.maxLights = 32;
	resources.maxClipPlanes = 6;
	resources.maxTextureUnits = 32;
	resources.maxTextureCoords = 32;
	resources.maxVertexAttribs = 64;
	resources.maxVertexUniformComponents = 4096;
	resources.maxVaryingFloats = 64;
	resources.maxVertexTextureImageUnits = 32;
	resources.maxCombinedTextureImageUnits = 80;
	resources.maxTextureImageUnits = 32;
	resources.maxFragmentUniformComponents = 4096;
	resources.maxDrawBuffers = 32;
	resources.maxVertexUniformVectors = 128;
	resources.maxVaryingVectors = 8;
	resources.maxFragmentUniformVectors = 16;
	resources.maxVertexOutputVectors = 16;
	resources.maxFragmentInputVectors = 15;
	resources.minProgramTexelOffset = -8;
	resources.maxProgramTexelOffset = 7;
	resources.maxClipDistances = 8;
	resources.maxComputeWorkGroupCountX = 65535;
	resources.maxComputeWorkGroupCountY = 65535;
	resources.maxComputeWorkGroupCountZ = 65535;
	resources.maxComputeWorkGroupSizeX = 1024;
	resources.maxComputeWorkGroupSizeY = 1024;
	resources.maxComputeWorkGroupSizeZ = 64;
	resources.maxComputeUniformComponents = 1024;
	resources.maxComputeTextureImageUnits = 16;
	resources.maxComputeImageUniforms = 8;
	resources.maxComputeAtomicCounters = 8;
	resources.maxComputeAtomicCounterBuffers = 1;
	resources.maxVaryingComponents = 60;
	resources.maxVertexOutputComponents = 64;
	resources.maxGeometryInputComponents = 64;
	resources.maxGeometryOutputComponents = 128;
	resources.maxFragmentInputComponents = 128;
	resources.maxImageUnits = 8;
	resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	resources.maxCombinedShaderOutputResources = 8;
	resources.maxImageSamples = 0;
	resources.maxVertexImageUniforms = 0;
	resources.maxTessControlImageUniforms = 0;
	resources.maxTessEvaluationImageUniforms = 0;
	resources.maxGeometryImageUniforms = 0;
	resources.maxFragmentImageUniforms = 8;
	resources.maxCombinedImageUniforms = 8;
	resources.maxGeometryTextureImageUnits = 16;
	resources.maxGeometryOutputVertices = 256;
	resources.maxGeometryTotalOutputComponents = 1024;
	resources.maxGeometryUniformComponents = 1024;
	resources.maxGeometryVaryingComponents = 64;
	resources.maxTessControlInputComponents = 128;
	resources.maxTessControlOutputComponents = 128;
	resources.maxTessControlTextureImageUnits = 16;
	resources.maxTessControlUniformComponents = 1024;
	resources.maxTessControlTotalOutputComponents = 4096;
	resources.maxTessEvaluationInputComponents = 128;
	resources.maxTessEvaluationOutputComponents = 128;
	resources.maxTessEvaluationTextureImageUnits = 16;
	resources.maxTessEvaluationUniformComponents = 1024;
	resources.maxTessPatchComponents = 120;
	resources.maxPatchVertices = 32;
	resources.maxTessGenLevel = 64;
	resources.maxViewports = 16;
	resources.maxVertexAtomicCounters = 0;
	resources.maxTessControlAtomicCounters = 0;
	resources.maxTessEvaluationAtomicCounters = 0;
	resources.maxGeometryAtomicCounters = 0;
	resources.maxFragmentAtomicCounters = 8;
	resources.maxCombinedAtomicCounters = 8;
	resources.maxAtomicCounterBindings = 1;
	resources.maxVertexAtomicCounterBuffers = 0;
	resources.maxTessControlAtomicCounterBuffers = 0;
	resources.maxTessEvaluationAtomicCounterBuffers = 0;
	resources.maxGeometryAtomicCounterBuffers = 0;
	resources.maxFragmentAtomicCounterBuffers = 1;
	resources.maxCombinedAtomicCounterBuffers = 1;
	resources.maxAtomicCounterBufferSize = 16384;
	resources.maxTransformFeedbackBuffers = 4;
	resources.maxTransformFeedbackInterleavedComponents = 64;
	resources.maxCullDistances = 8;
	resources.maxCombinedClipAndCullDistances = 8;
	resources.maxSamples = 4;
	resources.maxMeshOutputVerticesNV = 256;
	resources.maxMeshOutputPrimitivesNV = 512;
	resources.maxMeshWorkGroupSizeX_NV = 32;
	resources.maxMeshWorkGroupSizeY_NV = 1;
	resources.maxMeshWorkGroupSizeZ_NV = 1;
	resources.maxTaskWorkGroupSizeX_NV = 32;
	resources.maxTaskWorkGroupSizeY_NV = 1;
	resources.maxTaskWorkGroupSizeZ_NV = 1;
	resources.maxMeshViewCountNV = 4;
	resources.limits.nonInductiveForLoops = 1;
	resources.limits.whileLoops = 1;
	resources.limits.doWhileLoops = 1;
	resources.limits.generalUniformIndexing = 1;
	resources.limits.generalAttributeMatrixVectorIndexing = 1;
	resources.limits.generalVaryingIndexing = 1;
	resources.limits.generalSamplerIndexing = 1;
	resources.limits.generalVariableIndexing = 1;
	resources.limits.generalConstantMatrixVectorIndexing = 1;

	return resources;
}();




bool BvSPIRVCompiler::Compile(const u8* const pBlob, const size_t blobSize, const ShaderDesc& shaderDesc,
	std::vector<u32>& compiledSpirv, BvStringT* const pErrors)
{
	auto shaderStage = GetEShShaderStage(shaderDesc.shaderStage);
	glslang::TShader shader(shaderStage);
	shader.setEnvInput(glslang::EShSource::EShSourceGlsl, shaderStage, glslang::EShClient::EShClientVulkan, 100);
	shader.setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_2);
	shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_5);
	shader.setEntryPoint(shaderDesc.entryPoint.CStr());

	const char* shaderStrings[] = { reinterpret_cast<const char*>(pBlob) };
	i32 lenghts[] = { static_cast<i32>(blobSize) };
	shader.setStringsWithLengths(shaderStrings, lenghts, 1);
	//shader.setAutoMapBindings(true);

	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
	if (!shader.parse(&g_BuiltInResources, 100, false, messages))
	{
		if (pErrors)
		{
			pErrors->Append(shader.getInfoLog());
		}
		return false;
	}

	glslang::TProgram program;
	program.addShader(&shader);
	if (!program.link(messages) || !program.mapIO())
	{
		if (pErrors)
		{
			pErrors->Append(program.getInfoLog());
		}
		return false;
	}

	std::vector<u32> spv;
	glslang::GlslangToSpv(*program.getIntermediate(shader.getStage()), spv);

	spvtools::Optimizer optimizer(spv_target_env::SPV_ENV_VULKAN_1_2);
	optimizer.RegisterPerformancePasses();
	if (!optimizer.Run(spv.data(), spv.size(), &compiledSpirv))
	{
		if (pErrors)
		{
			pErrors->Append("Error optimizing spirv binary.");
		}
		return false;
	}

	return true;
}
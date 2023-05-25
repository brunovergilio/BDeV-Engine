#include "BvSPIRVCompiler.h"


#if (BV_PLATFORM == BV_PLATFORM_WIN32 && BV_COMPILER == BV_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable:26451 26495)
#endif


#include "External/glslang/glslang/Include/glslang_c_interface.h"
#include "External/glslang/glslang/Public/ResourceLimits.h"
#include "External/glslang/glslang/Public/resource_limits_c.h"



class GlslangControl
{
public:
	GlslangControl()
	{
		glslang_initialize_process();
	}

	~GlslangControl()
	{
		glslang_finalize_process();
	}
} g_GlslangControl;


glslang_stage_t GetShaderStage(ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::kVertex:				return glslang_stage_t::GLSLANG_STAGE_VERTEX;
	case ShaderStage::kHullOrControl:		return glslang_stage_t::GLSLANG_STAGE_TESSCONTROL;
	case ShaderStage::kDomainOrEvaluation:	return glslang_stage_t::GLSLANG_STAGE_TESSEVALUATION;
	case ShaderStage::kGeometry:			return glslang_stage_t::GLSLANG_STAGE_GEOMETRY;
	case ShaderStage::kPixelOrFragment:		return glslang_stage_t::GLSLANG_STAGE_FRAGMENT;
	case ShaderStage::kMesh:				return glslang_stage_t::GLSLANG_STAGE_MESH;
	case ShaderStage::kAmplificationOrTask:	return glslang_stage_t::GLSLANG_STAGE_TASK;
	case ShaderStage::kCompute:				return glslang_stage_t::GLSLANG_STAGE_COMPUTE;
	default:
		BvAssert(0, "This code should be unreachable");
		return glslang_stage_t::GLSLANG_STAGE_COUNT;
	}
}


glslang_target_client_version_t GetAPIVersion(ShaderAPIVersion apiVersion)
{
	switch (apiVersion)
	{
	//case ShaderAPIVersion::kOpenGL_4_5:	return glslang_target_client_version_t::GLSLANG_TARGET_OPENGL_450;
	case ShaderAPIVersion::kVulkan_1_0:	return glslang_target_client_version_t::GLSLANG_TARGET_VULKAN_1_0;
	case ShaderAPIVersion::kVulkan_1_1:	return glslang_target_client_version_t::GLSLANG_TARGET_VULKAN_1_1;
	case ShaderAPIVersion::kVulkan_1_2:	return glslang_target_client_version_t::GLSLANG_TARGET_VULKAN_1_2;
	case ShaderAPIVersion::kVulkan_1_3:	return glslang_target_client_version_t::GLSLANG_TARGET_VULKAN_1_3;
	default:
		BvAssert(0, "This code should be unreachable");
		return glslang_target_client_version_t::GLSLANG_TARGET_CLIENT_VERSION_COUNT;
	}
}


glslang_target_language_version_t GetShaderTarget(ShaderLanguageTarget target)
{
	switch (target)
	{
	case ShaderLanguageTarget::kSPIRV_1_0: return glslang_target_language_version_t::GLSLANG_TARGET_SPV_1_0;
	case ShaderLanguageTarget::kSPIRV_1_1: return glslang_target_language_version_t::GLSLANG_TARGET_SPV_1_1;
	case ShaderLanguageTarget::kSPIRV_1_2: return glslang_target_language_version_t::GLSLANG_TARGET_SPV_1_2;
	case ShaderLanguageTarget::kSPIRV_1_3: return glslang_target_language_version_t::GLSLANG_TARGET_SPV_1_3;
	case ShaderLanguageTarget::kSPIRV_1_4: return glslang_target_language_version_t::GLSLANG_TARGET_SPV_1_4;
	case ShaderLanguageTarget::kSPIRV_1_5: return glslang_target_language_version_t::GLSLANG_TARGET_SPV_1_5;
	case ShaderLanguageTarget::kSPIRV_1_6: return glslang_target_language_version_t::GLSLANG_TARGET_SPV_1_6;
	default:
		BvAssert(0, "This code should be unreachable");
		return glslang_target_language_version_t::GLSLANG_TARGET_LANGUAGE_VERSION_COUNT;
	}
}


void BvSPIRVCompiler::Compile(const u8* const pBlob, const size_t blobSize, const ShaderDesc& shaderDesc,
	BvVector<u8>& compiledShaderBlob, BvString& errorString)
{
	auto stage = GetShaderStage(shaderDesc.shaderStage);
	const glslang_input_t input =
	{
		.language = GLSLANG_SOURCE_GLSL,
		.stage = stage,
		.client = GLSLANG_CLIENT_VULKAN,
		.client_version = GetAPIVersion(shaderDesc.apiVersion),
		.target_language = GLSLANG_TARGET_SPV,
		.target_language_version = GetShaderTarget(shaderDesc.shaderTarget),
		.code = reinterpret_cast<const char*>(pBlob),
		.default_version = 100,
		.default_profile = GLSLANG_NO_PROFILE,
		.force_default_version_and_profile = false,
		.forward_compatible = false,
		.messages = GLSLANG_MSG_DEFAULT_BIT,
		.resource = glslang_default_resource(),
	};


	glslang_shader_t* pShader = glslang_shader_create(&input);
	if (!glslang_shader_preprocess(pShader, &input))
	{
		printf("%s\n", glslang_shader_get_info_log(pShader));
		printf("%s\n", glslang_shader_get_info_debug_log(pShader));
		printf("%s\n", input.code);
		glslang_shader_delete(pShader);
		return;
	}

	if (!glslang_shader_parse(pShader, &input))
	{
		printf("%s\n", glslang_shader_get_info_log(pShader));
		printf("%s\n", glslang_shader_get_info_debug_log(pShader));
		printf("%s\n", glslang_shader_get_preprocessed_code(pShader));
		glslang_shader_delete(pShader);
		return;
	}

	glslang_program_t* pProgram = glslang_program_create();
	glslang_program_add_shader(pProgram, pShader);

	if (!glslang_program_link(pProgram, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
	{
		printf("%s\n", glslang_program_get_info_log(pProgram));
		printf("%s\n", glslang_program_get_info_debug_log(pProgram));
		glslang_program_delete(pProgram);
		glslang_shader_delete(pShader);
		return;
	}

	glslang_program_SPIRV_generate(pProgram, stage);

	compiledShaderBlob.Resize(glslang_program_SPIRV_get_size(pProgram) * sizeof(u32));
	glslang_program_SPIRV_get(pProgram, (u32*)compiledShaderBlob.Data());
}


#if (BV_PLATFORM == BV_PLATFORM_WIN32 && BV_COMPILER == BV_COMPILER_MSVC)
#pragma warning(pop)
#endif
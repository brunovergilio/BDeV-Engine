#pragma once


#include "BvRenderTools/BvShaderCompiler.h"


class BvSPIRVReflector final
{
public:
	BvSPIRVReflector() {}
	~BvSPIRVReflector() {}

	IBvShaderReflectionData* Reflect(const u8* pBlob, const u32 blobSize, ShaderLanguage shaderLanguage) const;
};
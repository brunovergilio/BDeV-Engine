#pragma once


#include "BvRenderTools/ShaderCompiler/BvShaderCompiler.h"
#include <vector>


class BvSPIRVCompiler
{
public:
	BvSPIRVCompiler() {}
	~BvSPIRVCompiler() {}

	bool Compile(const u8* const pBlob, const size_t blobSize, const ShaderDesc& shaderDesc,
		BvVector<u8>& compiledShaderBlob, BvString* pErrorString);
};
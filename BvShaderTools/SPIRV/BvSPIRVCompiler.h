#pragma once


#include "BvShaderTools/BvShaderCompiler.h"
#include <vector>


class BvSPIRVCompiler
{
public:
	BvSPIRVCompiler() {}
	~BvSPIRVCompiler() {}

	bool Compile(const u8* const pBlob, const size_t blobSize, const ShaderDesc& shaderDesc,
		std::vector<u32>& compiledSpirv, BvString* const pErrors = nullptr);
};
#pragma once


#include "BvRenderTools/BvShaderCompiler.h"
#include <vector>


class BvSPIRVCompiler
{
public:
	BvSPIRVCompiler() {}
	~BvSPIRVCompiler() {}

	void Compile(const u8* const pBlob, const size_t blobSize, const ShaderDesc& shaderDesc,
		BvVector<u8>& compiledShaderBlob, BvString& errorString);
};
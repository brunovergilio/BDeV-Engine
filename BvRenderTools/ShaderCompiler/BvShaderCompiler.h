#pragma once

#include "BvRenderTools/ShaderCompiler/BvShaderBlob.h"
#include "BDeV/Core/System/File/BvPath.h"


class BvShaderCompiler final : public IBvShaderCompiler
{
public:
	BvShaderCompiler();
	~BvShaderCompiler();

private:
	bool CompileImpl(const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob = nullptr) override;
	bool CompileFromFileImpl(const char* pFilename, const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob = nullptr) override;
};
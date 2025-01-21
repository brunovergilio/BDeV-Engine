#pragma once


#include "BDeV/Core/RenderAPI/BvShaderCompiler.h"
#include "BDeV/Core/Container/BvVector.h"


class BvShaderBlob final : public IBvShaderBlob
{
	BV_NOCOPYMOVE(BvShaderBlob);

public:
	BvShaderBlob(BvVector<u8>& shaderBlob);
	BvShaderBlob(const char* pErrorString);
	~BvShaderBlob();

	BV_INLINE const void* GetBufferPointer() const override { return m_ShaderBlob.Data(); }
	BV_INLINE u64 GetBufferSize() const override { return m_ShaderBlob.Size(); }

private:
	BvVector<u8> m_ShaderBlob;
};
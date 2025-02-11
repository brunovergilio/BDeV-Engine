#pragma once


#include "BDeV/Core/RenderAPI/BvShaderCompiler.h"
#include "BDeV/Core/Container/BvVector.h"


BV_OBJECT_DEFINE_ID(BvShaderBlob, "fce3fec8-8e9a-4d7f-8da8-39953d8a7471");
class BvShaderBlob final : public IBvShaderBlob
{
	BV_NOCOPYMOVE(BvShaderBlob);

public:
	BvShaderBlob(BvVector<u8>& shaderBlob);
	BvShaderBlob(const char* pErrorString);
	~BvShaderBlob();

	BV_INLINE const void* GetBufferPointer() const override { return m_ShaderBlob.Data(); }
	BV_INLINE u64 GetBufferSize() const override { return m_ShaderBlob.Size(); }

	BV_OBJECT_IMPL_INTERFACE(BvShaderBlob, IBvShaderBlob);

private:
	BvVector<u8> m_ShaderBlob;
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvShaderBlob);
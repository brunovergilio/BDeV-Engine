#pragma once


#include "BDeV/Core/RenderAPI/BvShaderCompiler.h"
#include "BDeV/Core/Container/BvVector.h"


BV_IBVOBJECT_CREATE_REFCOUNT_BASE(BvShaderBlobRefCount, IBvShaderBlob);


class BvShaderBlob final : public BvShaderBlobRefCount
{
	BV_NOCOPYMOVE(BvShaderBlob);

public:
	BV_IBVOBJECT_IMPL_INTERFACE(BvShaderBlob, IBvShaderBlob);

	BvShaderBlob(BvVector<u8>& shaderBlob);
	BvShaderBlob(const char* pErrorString);
	~BvShaderBlob();

	BV_INLINE const void* GetBufferPointer() const override { return m_ShaderBlob.Data(); }
	BV_INLINE u64 GetBufferSize() const override { return m_ShaderBlob.Size(); }

private:
	BvVector<u8> m_ShaderBlob;
};
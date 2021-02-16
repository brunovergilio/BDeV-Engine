#include "BvShaderCompiler.h"
#include "BvCore/System/File/BvFile.h"
#include "BvCore/System/File/BvFileSystem.h"
#include "SPIRV/BvSPIRVCompiler.h"
#include <vector>


class BvShaderBlob final : public IBvShaderBlob
{
	BV_NOCOPYMOVE(BvShaderBlob);

public:
	BvShaderBlob(u8* const pBlob, const size_t blobSize)
		: m_pBlob(new u8[blobSize]), m_BlobSize(blobSize)
	{
		memcpy(m_pBlob, pBlob, blobSize);
	}

	~BvShaderBlob() { BvDeleteArray(m_pBlob); }

	BV_INLINE const u8* const GetBufferPointer() const override { return m_pBlob; }
	BV_INLINE const size_t GetBufferSize() const override { return m_BlobSize; }

private:
	u8* m_pBlob = nullptr;
	size_t m_BlobSize = 0;
};



class BvShaderCompiler final : public IBvShaderCompiler
{
	BV_NOCOPYMOVE(BvShaderCompiler);

public:
	BvShaderCompiler();
	~BvShaderCompiler();

	IBvShaderBlob* CompileFromFile(const char* const pFilename, const ShaderDesc& shaderDesc,
		BvString* const pErrors) const override;
	IBvShaderBlob* CompileFromMemory(const u8* const pBlob, const size_t blobSize, const ShaderDesc& shaderDesc,
		BvString* const pErrors) const override;
	void DestroyShader(IBvShaderBlob*& pCompiledShader) const override;
};


BvShaderCompiler::BvShaderCompiler()
{
}


BvShaderCompiler::~BvShaderCompiler()
{
}


IBvShaderBlob* BvShaderCompiler::CompileFromFile(const char * const pFilename, const ShaderDesc& shaderDesc,
	BvString* const pErrors) const
{
	BvFileSystem fileSys;
	BvFile file = fileSys.OpenFile(pFilename, BvFileAccess::kRead);
	BvAssert(file.IsValid());

	auto size = file.GetSize();
	auto pBlob = new u8[size];
	file.Read(pBlob, size);

	auto result = CompileFromMemory(pBlob, size, shaderDesc, pErrors);

	BvDeleteArray(pBlob);

	return result;
}


IBvShaderBlob* BvShaderCompiler::CompileFromMemory(const u8* const pBlob, const size_t blobSize, const ShaderDesc& shaderDesc,
	BvString* const pErrors) const
{
	BvShaderBlob* pCompiledShader = nullptr;
	switch (shaderDesc.shaderLanguage)
	{
	case ShaderLanguage::kGLSL:
	{
		BvSPIRVCompiler compiler;
		std::vector<u32> spirv;
		if (compiler.Compile(pBlob, blobSize, shaderDesc, spirv, pErrors))
		{
			pCompiledShader = new BvShaderBlob(reinterpret_cast<u8*>(spirv.data()), spirv.size() * sizeof(u32));
		}
		break;
	}
	}

	return pCompiledShader;
}


void BvShaderCompiler::DestroyShader(IBvShaderBlob*& pCompiledShader) const
{
	auto pPtr = reinterpret_cast<BvShaderBlob*>(pCompiledShader);
	delete pPtr;
	pCompiledShader = nullptr;
}


IBvShaderCompiler* CreateShaderCompiler()
{
	return new BvShaderCompiler();
}


void DestroyShaderCompiler(IBvShaderCompiler*& pCompiler)
{
	auto pPtr = reinterpret_cast<BvShaderCompiler*>(pCompiler);
	delete pPtr;
	pCompiler = nullptr;
}
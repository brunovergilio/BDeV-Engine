#include "BvShaderCompiler.h"
#include "BDeV/System/File/BvFile.h"
#include "SPIRV/BvSPIRVCompiler.h"
#include <vector>


class BvShaderBlob final : public IBvShaderBlob
{
	BV_NOCOPYMOVE(BvShaderBlob);

public:
	BvShaderBlob(BvVector<u8>&& shaderBlob, BvString&& errorString)
		: m_ShaderBlob(std::move(shaderBlob)), m_ErrorString(std::move(errorString))
	{
	}
	~BvShaderBlob() {}

	BV_INLINE const BvVector<u8>& GetShaderBlob() const override { return m_ShaderBlob; }
	BV_INLINE const BvString& GetErrorString() const override { return m_ErrorString; }
	BV_INLINE bool IsValid() const override { return m_ShaderBlob.Size() > 0; }

private:
	BvVector<u8> m_ShaderBlob;
	BvString m_ErrorString;
};



class BvShaderCompiler final : public IBvShaderCompiler
{
	BV_NOCOPYMOVE(BvShaderCompiler);

public:
	BvShaderCompiler();
	~BvShaderCompiler();

	IBvShaderBlob* CompileFromFile(const char* const pFilename, const ShaderDesc& shaderDesc) override;
	IBvShaderBlob* CompileFromMemory(const u8* const pBlob, const size_t blobSize, const ShaderDesc& shaderDesc) override;
	void DestroyShader(IBvShaderBlob*& pCompiledShader) override;
	void DestroyAllShaders() override;

private:
	BvVector<BvShaderBlob*> m_ShaderBlobs;
};


BvShaderCompiler::BvShaderCompiler()
{
}


BvShaderCompiler::~BvShaderCompiler()
{
	DestroyAllShaders();
}


IBvShaderBlob* BvShaderCompiler::CompileFromFile(const char * const pFilename, const ShaderDesc& shaderDesc)
{
	BvFile file(pFilename, BvFileAccessMode::kRead, BvFileAction::kOpen);
	if (!file.IsValid())
	{
		return nullptr;
	}

	auto size = file.GetSize();
	auto pBlob = new u8[size];
	file.Read(pBlob, size);

	auto result = CompileFromMemory(pBlob, size, shaderDesc);

	delete[] pBlob;

	return result;
}


IBvShaderBlob* BvShaderCompiler::CompileFromMemory(const u8* const pBlob, const size_t blobSize, const ShaderDesc& shaderDesc)
{
	BvShaderBlob* pCompiledShader = nullptr;
	switch (shaderDesc.shaderLanguage)
	{
	case ShaderLanguage::kGLSL:
	{
		BvVector<u8> shaderBlob;
		BvString errorString;

		BvSPIRVCompiler compiler;
		compiler.Compile(pBlob, blobSize, shaderDesc, shaderBlob, errorString);
		pCompiledShader = new BvShaderBlob(std::move(shaderBlob), std::move(errorString));
		m_ShaderBlobs.PushBack(pCompiledShader);
		break;
	}
	}

	return pCompiledShader;
}


void BvShaderCompiler::DestroyShader(IBvShaderBlob*& pCompiledShader)
{
	auto pPtr = reinterpret_cast<BvShaderBlob*>(pCompiledShader);
	for (auto i = 0; i < m_ShaderBlobs.Size(); i++)
	{
		if (m_ShaderBlobs[i] == pPtr)
		{
			delete m_ShaderBlobs[i];
			m_ShaderBlobs.EraseAndSwapWithLast(i);
		}
	}
	pCompiledShader = nullptr;
}


void BvShaderCompiler::DestroyAllShaders()
{
	for (auto pShaderBlob : m_ShaderBlobs)
	{
		delete pShaderBlob;
	}
	m_ShaderBlobs.Clear();
}


namespace BvRenderTools
{
	IBvShaderCompiler* GetShaderCompiler()
	{
		static BvShaderCompiler s_ShaderCompiler;
		return &s_ShaderCompiler;
	}
}
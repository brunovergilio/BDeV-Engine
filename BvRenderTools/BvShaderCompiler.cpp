#include "BvShaderCompiler.h"
#include "BDeV/System/File/BvFile.h"
#include "SPIRV/BvSPIRVCompiler.h"
#include "BDeV/Container/BvRobinSet.h"
#include "BDeV/System/File/BvPath.h"


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
	IBvShaderBlob* CompileFromMemory(const u8* const pBlob, u32 blobSize, const ShaderDesc& shaderDesc) override;

	void DestroyShader(IBvShaderBlob*& pCompiledShader) override;
	void DestroyAllShaders() override;

private:
	void ProcessIncludes(const char* pFirst, u32 size, BvString& result, const BvPath& basePath);

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
	BvPath filePath(pFilename);
	if (!filePath.IsAbsolute())
	{
		filePath.ConvertToAbsolutePath();
	}

	auto file = filePath.AsFile(BvFileAccessMode::kRead);
	if (!file.IsValid())
	{
		return nullptr;
	}

	auto size = file.GetSize();
	BvString blob((u32)size);
	file.Read(&blob[0], size);
	file.Close();

	filePath.MoveToParentPath();
	BvString preprocessedShader;
	ProcessIncludes(blob.CStr(), blob.Size(), preprocessedShader, filePath);

	auto result = CompileFromMemory((const u8*)preprocessedShader.CStr(), size, shaderDesc);

	return result;
}


IBvShaderBlob* BvShaderCompiler::CompileFromMemory(const u8* const pBlob, u32 blobSize, const ShaderDesc& shaderDesc)
{
	BvShaderBlob* pCompiledShader = nullptr;
	switch (shaderDesc.shaderLanguage)
	{
	case ShaderLanguage::kVKSL:
	case ShaderLanguage::kGLSL:
	{
		BvVector<u8> shaderBlob;
		BvString errorString;

		BvSPIRVCompiler compiler;
		compiler.Compile(pBlob, blobSize, shaderDesc, shaderBlob, &errorString);
		pCompiledShader = new BvShaderBlob(std::move(shaderBlob), std::move(errorString));
		m_ShaderBlobs.PushBack(pCompiledShader);
		break;
	}
	case ShaderLanguage::kHLSL:
	{
		// TODO: Implement FXC / DXC code
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


const char* SkipDelimitersAndComments(const char* pCurr, const char* pLast)
{
	while (pCurr != pLast && (std::isspace(*pCurr) || *pCurr == '/'))
	{
		if (*pCurr == '/')
		{
			++pCurr;
			if (*pCurr == '/')
			{
				++pCurr;
				while (pCurr != pLast && *pCurr != '\n')
				{
					++pCurr;
				}
			}
			else if (*pCurr == '*')
			{
				++pCurr;
				if (pCurr != pLast)
				{
					while (pCurr + 1 != pLast)
					{
						if (pCurr[0] == '*' && pCurr[1] == '/')
						{
							pCurr += 2;
							break;
						}
						++pCurr;
					}
				}
			}
		}
		else
		{
			++pCurr;
		}
	}

	return pCurr;
}


bool IsIncludeChar(char c)
{
	return c == '<' || c == '>' || c == '\'' || c == '\"';
}


void BvShaderCompiler::ProcessIncludes(const char* pFirst, u32 size, BvString& result, const BvPath& basePath)
{
	const char* pCurr = pFirst;
	const char* pLast = pFirst + size;
	const char* pLastWrite = pCurr;
	while (pCurr != pLast)
	{
		pCurr = SkipDelimitersAndComments(pCurr, pLast);
		if (*pCurr != '#')
		{
			if (pCurr != pLast)
			{
				++pCurr;
			}
			continue;
		}
		
		auto pHash = pCurr++;
		pCurr = SkipDelimitersAndComments(pCurr, pLast);
		auto pKeywordEnd = pCurr;
		while (pKeywordEnd != pLast && !std::isspace(*pKeywordEnd))
		{
			++pKeywordEnd;
		}

		if (u32(pKeywordEnd - pCurr) != 7 || strnicmp(pCurr, "include", 7) != 0)
		{
			pCurr = pKeywordEnd;
			continue;
		}

		// If we got here we have found '#include', so append all the code up to this point
		result.Append(pLastWrite, 0, u32(pHash - pLastWrite));

		pCurr = pKeywordEnd;
		pCurr = SkipDelimitersAndComments(pCurr, pLast);

		if (IsIncludeChar(*pCurr))
		{
			auto pPathFirst = ++pCurr;
			while (pCurr != pLast && !IsIncludeChar(*pCurr))
			{
				++pCurr;
			}

			if (pCurr != pLast)
			{
				auto pPathLast = pCurr;

				BvPath path(pPathFirst, u32(pPathLast - pPathFirst));
				if (!path.IsAbsolute())
				{
					path.PrependPath(basePath);
				}
				path.NormalizePath();

				auto file = path.AsFile(BvFileAccessMode::kRead);
				if (file.IsValid())
				{
					auto fileSize = file.GetSize();
					BvString blob((u32)fileSize);
					file.Read(&blob[0], fileSize);
					file.Close();

					path.MoveToParentPath();

					ProcessIncludes(blob.CStr(), blob.Size(), result, path);
				}
			}
		}

		// At this point we're either at the end or at the include character
		if (pCurr != pLast)
		{
			++pCurr;
		}

		pLastWrite = pCurr;
	}

	if (pLastWrite != pLast)
	{
		result.Append(pLastWrite, 0, u32(pLast - pLastWrite));
	}
}


namespace BvRenderTools
{
	IBvShaderCompiler* GetShaderCompiler()
	{
		static BvShaderCompiler s_ShaderCompiler;
		return &s_ShaderCompiler;
	}
}
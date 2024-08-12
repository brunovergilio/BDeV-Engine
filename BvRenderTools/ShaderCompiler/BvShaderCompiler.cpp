#include "BvShaderCompiler.h"
#include "BDeV/System/File/BvFile.h"
#include "SPIRV/BvSPIRVCompiler.h"
#include "BDeV/Container/BvRobinSet.h"
#include "BDeV/System/File/BvPath.h"


class BvShaderBlob final : public IBvShaderBlob
{
	BV_NOCOPYMOVE(BvShaderBlob);

public:
	BvShaderBlob(BvVector<u8>& shaderBlob)
		: m_ShaderBlob(std::move(shaderBlob)) {}
	~BvShaderBlob() {}

	BV_INLINE const u8* GetBufferPointer() const override { return m_ShaderBlob.Data(); }
	BV_INLINE u64 GetBufferSize() const override { return m_ShaderBlob.Size(); }

private:
	BvVector<u8> m_ShaderBlob;
};



class BvShaderCompiler final : public IBvShaderCompiler
{
	BV_NOCOPYMOVE(BvShaderCompiler);

public:
	BvShaderCompiler();
	~BvShaderCompiler();

	bool CompileFromFile(const char* const pFilename, const ShaderDesc& shaderDesc, IBvShaderBlob*& pShaderBlob, BvString* pErrorString = nullptr) override;
	bool CompileFromMemory(const u8* const pBlob, u32 blobSize, const ShaderDesc& shaderDesc, IBvShaderBlob*& pShaderBlob, BvString* pErrorString = nullptr) override;

	void DestroyShader(IBvShaderBlob*& pShaderBlob) override;

private:
	void ProcessIncludes(const char* pFirst, u32 size, BvString& result, const BvPath& basePath);

private:
	BvRobinSet<BvShaderBlob*> m_Shaders;
};


BvShaderCompiler::BvShaderCompiler()
{
}


BvShaderCompiler::~BvShaderCompiler()
{
	for (auto pBlob : m_Shaders)
	{
		delete pBlob;
	}
}


bool BvShaderCompiler::CompileFromFile(const char * const pFilename, const ShaderDesc& shaderDesc, IBvShaderBlob*& pShaderBlob, BvString* pErrorString)
{
	BvPath filePath(pFilename);
	if (!filePath.IsAbsolute())
	{
		filePath.ConvertToAbsolutePath();
	}

	auto file = filePath.AsFile(BvFileAccessMode::kRead);
	if (!file.IsValid())
	{
		return false;
	}

	auto size = file.GetSize();
	BvString blob((u32)size);
	file.Read(&blob[0], size);
	file.Close();

	filePath.MoveToParentPath();
	BvString preprocessedShader;
	ProcessIncludes(blob.CStr(), blob.Size(), preprocessedShader, filePath);

	auto result = CompileFromMemory((const u8*)preprocessedShader.CStr(), size, shaderDesc, pShaderBlob, pErrorString);

	return result;
}


bool BvShaderCompiler::CompileFromMemory(const u8* const pBlob, u32 blobSize, const ShaderDesc& shaderDesc, IBvShaderBlob*& pShaderBlob, BvString* pErrorString)
{
	switch (shaderDesc.shaderLanguage)
	{
	case ShaderLanguage::kVKSL:
	case ShaderLanguage::kGLSL:
	{
		BvVector<u8> compiledShader;

		BvSPIRVCompiler compiler;
		if (compiler.Compile(pBlob, blobSize, shaderDesc, compiledShader, pErrorString))
		{
			auto pResult = new BvShaderBlob(compiledShader);
			m_Shaders.Emplace(pResult);
			pShaderBlob = pResult;
		}
		break;
	}
	case ShaderLanguage::kHLSL:
	{
		// TODO: Implement FXC / DXC code
		break;
	}
	}

	return pShaderBlob != nullptr;
}


void BvShaderCompiler::DestroyShader(IBvShaderBlob*& pShaderBlob)
{
	auto pPtr = static_cast<BvShaderBlob*>(pShaderBlob);
	delete pPtr;

	pShaderBlob = nullptr;
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
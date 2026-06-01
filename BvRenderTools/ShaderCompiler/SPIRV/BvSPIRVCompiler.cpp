#include "BvSPIRVCompiler.h"
#include "BvSPIRVUtils.h"


BvSPIRVCompiler::BvSPIRVCompiler()
{
}


BvSPIRVCompiler::~BvSPIRVCompiler()
{
}


bool BvSPIRVCompiler::CompileImpl(const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob)
{
	BvVector<u8> compiledShaderBlob;
	BvVector<u8> errorMessage;
	if (!BvSPIRVUtils::CompileSPIRV(shaderDesc, compiledShaderBlob, errorMessage))
	{
		if (ppErrorBlob)
		{
			*ppErrorBlob = BV_RC_CREATE(BvShaderBlob, errorMessage);
		}

		return false;
	}

	*ppShaderBlob = BV_RC_CREATE(BvShaderBlob, compiledShaderBlob);

	return true;
}


bool BvSPIRVCompiler::CompileFromFileImpl(const char* pFilename, const ShaderSourceDesc& shaderDesc, void** ppShaderBlob, void** ppErrorBlob)
{
	BvFile file(pFilename, BvFileAccessMode::kRead, BvFileAction::kOpen);
	if (!file.IsValid())
	{
		if (ppErrorBlob)
		{
			*ppErrorBlob = BV_RC_CREATE(BvShaderBlob, "Error opening the file.");
		}
		return false;
	}
	auto size = file.GetSize();
	BvVector<char> sourceCode(size);
	file.Read(sourceCode.Data(), size);

	auto& desc = const_cast<ShaderSourceDesc&>(shaderDesc);
	desc.m_pSourceCode = sourceCode.Data();
	desc.m_SourceCodeSize = size;

	return CompileImpl(shaderDesc, ppShaderBlob, ppErrorBlob);
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


void BvSPIRVCompiler::ProcessIncludes(const char* pFirst, u32 size, BvString& result, const BvPath& basePath)
{
	if (!basePath.IsValid())
	{
		return;
	}

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
	extern "C"
	{
		BV_API bool CreateSPIRVCompiler(void** ppObj)
		{
			*ppObj = BV_RC_CREATE(BvSPIRVCompiler);
			return true;
		}
	}
}
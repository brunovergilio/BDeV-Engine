#include "BDeV/Core/System/Library/BvSharedLib.h"
#include "BDeV/Core/Container/BvText.h"
#include "BDeV/Core/System/Memory/BvMemoryArea.h"


BvSharedLib::BvSharedLib()
{
}


BvSharedLib::BvSharedLib(const char * pFilename)
{
	{
		auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(pFilename, 0, nullptr, 0);
		wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
		BvTextUtilities::ConvertUTF8CharToWideChar(pFilename, 0, pFilenameW, sizeNeeded);
		m_hLib = LoadLibraryW(pFilenameW);
	}
}


BvSharedLib::BvSharedLib(BvSharedLib && rhs) noexcept
{
	*this = std::move(rhs);
}


BvSharedLib & BvSharedLib::operator=(BvSharedLib && rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_hLib, rhs.m_hLib);
	}

	return *this;
}


BvSharedLib::~BvSharedLib()
{
	FreeLibrary((HMODULE)m_hLib);
}


void * BvSharedLib::GetProcAddress(const char * const pFunctionName) const
{
	return ::GetProcAddress((HMODULE)m_hLib, pFunctionName);
}
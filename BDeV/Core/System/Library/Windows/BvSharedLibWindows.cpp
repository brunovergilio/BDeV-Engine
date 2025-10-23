#include "BDeV/Core/System/Library/BvSharedLib.h"
#include "BDeV/Core/Utils/BvText.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include <utility>


BvSharedLib::BvSharedLib()
{
}


BvSharedLib::BvSharedLib(const char * pFilename)
{
	Open(pFilename);
}


BvSharedLib::BvSharedLib(BvSharedLib && rhs) noexcept
	: m_hLib(rhs.m_hLib)
{
	rhs.m_hLib = kNullOSSharedLibHandle;
}


BvSharedLib & BvSharedLib::operator=(BvSharedLib && rhs) noexcept
{
	if (this != &rhs)
	{
		Close();

		m_hLib = rhs.m_hLib;
		rhs.m_hLib = kNullOSSharedLibHandle;
	}

	return *this;
}


BvSharedLib::~BvSharedLib()
{
	Close();
}


bool BvSharedLib::Open(const char* pFilename)
{
	auto sizeNeeded = BvTextUtilities::ConvertUTF8CharToWideChar(pFilename, 0, nullptr, 0);
	wchar_t* pFilenameW = (wchar_t*)BV_STACK_ALLOC(sizeNeeded * sizeof(wchar_t));
	BvTextUtilities::ConvertUTF8CharToWideChar(pFilename, 0, pFilenameW, sizeNeeded);
	m_hLib = LoadLibraryW(pFilenameW);
	if (!m_hLib)
	{
		BV_SYS_ERROR();
		return false;
	}

	return true;
}


void BvSharedLib::Close()
{
	if (m_hLib)
	{
		FreeLibrary((HMODULE)m_hLib);
		m_hLib = nullptr;
	}
}


void * BvSharedLib::GetProcAddress(const char * const pFunctionName) const
{
	return ::GetProcAddress((HMODULE)m_hLib, pFunctionName);
}
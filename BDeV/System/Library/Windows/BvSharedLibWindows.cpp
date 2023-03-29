#include "BDeV/System/Library/Windows/BvSharedLibWindows.h"


BvSharedLib::BvSharedLib()
{
}


BvSharedLib::BvSharedLib(const char * const pFilename)
{
	m_hLib = LoadLibraryA(pFilename);
}


BvSharedLib::BvSharedLib(BvSharedLib && rhs) noexcept
{
	*this = std::move(rhs);
}


BvSharedLib & BvSharedLib::operator=(BvSharedLib && rhs) noexcept
{
	if (this != &rhs)
	{
		auto hDll = m_hLib;
		m_hLib = rhs.m_hLib;
		rhs.m_hLib = hDll;
	}

	return *this;
}


BvSharedLib::~BvSharedLib()
{
	FreeLibrary(m_hLib);
}


void * BvSharedLib::GetProcAddressInternal(const char * const pFunctionName) const
{
	return ::GetProcAddress(m_hLib, pFunctionName);
}
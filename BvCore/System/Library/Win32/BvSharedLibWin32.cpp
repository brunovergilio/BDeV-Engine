#include "BvCore/System/Library/Win32/BvSharedLibWin32.h"


BvSharedLib::BvSharedLib()
{
}


BvSharedLib::BvSharedLib(const char * const pFilename)
{
	m_hLib = LoadLibrary(pFilename);
}


BvSharedLib::BvSharedLib(BvSharedLib && rhs)
{
	*this = std::move(rhs);
}


BvSharedLib & BvSharedLib::operator=(BvSharedLib && rhs)
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
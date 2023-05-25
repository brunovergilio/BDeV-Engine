#include "BDeV/System/Library/BvSharedLib.h"
#include <Windows.h>


BvSharedLib::BvSharedLib()
{
}


BvSharedLib::BvSharedLib(const char * pFilename)
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
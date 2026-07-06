#include "BDeV/Core/System/Library/BvSharedLib.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include <utility>
#include "BDeV/Core/Utils/BvUTF.h"


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
	std::string_view sv(pFilename);
	auto length = BvUTFCharTraits::LengthFor<wchar_t>(sv.data(), sv.data() + sv.length() + 1);
	auto pFilenameW = (wchar_t*)BV_STACK_ALLOC(length * sizeof(wchar_t));
	BvUTFCharTraits::GetStr(sv.data(), sv.data() + sv.length() + 1, pFilenameW, pFilenameW + length);
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
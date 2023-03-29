#pragma once


#include "BDeV/BvCore.h"


constexpr const wchar_t* kWideFilePrefix = LR"(\\?\)";
constexpr size_t kWideFilePrefixSize = 4;

BV_API void ConvertToWidePath(wchar_t* pWidePath, const char* pPath);
BV_API void ConvertToWidePathWithPrefix(wchar_t* pWidePath, const char* pPath);
BV_API void AddPrefixForMaxPathLimit(wchar_t* pDstPath, const wchar_t* pSourcePath);
#pragma once


#include "BDeV/BvCore.h"


constexpr const wchar_t* kWideFilePrefix = LR"(\\?\)";
constexpr size_t kWideFilePrefixSize = 4;

void ConvertToWidePath(wchar_t* pWidePath, const char* pPath);
void ConvertToWidePathWithPrefix(wchar_t* pWidePath, const char* pPath);
void AddPrefixForMaxPathLimit(wchar_t* pDstPath, const wchar_t* pSourcePath);
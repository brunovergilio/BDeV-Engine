#pragma once


#include "BvCore/BvDefines.h"


u32 FNV1a32(const void * const pBytes, const u32 count);
u64 FNV1a64(const void * const pBytes, const u32 count);
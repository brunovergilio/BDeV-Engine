#pragma once


#include "../BvMathDefs.h"


#if (BV_MATH_USE_TYPE == BV_MATH_TYPE_SIMD)
#include "SIMD/BvFastQuat_SIMD.inl"
#elif (BV_MATH_USE_TYPE == BV_MATH_TYPE_FPU)
#include "FPU/BvFastQuat_FPU.inl"
#endif
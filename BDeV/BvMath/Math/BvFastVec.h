#pragma once


#include "../BvMathDefs.h"


#if (BV_MATH_USE_TYPE == BV_MATH_TYPE_SIMD)
#include "SIMD/BvFastVec_SIMD.inl"
#elif (BV_MATH_USE_TYPE == BV_MATH_TYPE_FPU)
#include "FPU/BvFastVec_FPU.inl"
#endif
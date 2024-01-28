#pragma once


#include "../BvMathDefs.h"


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_SIMD)
#include "SIMD/BvFastQuat_SIMD.inl"
#elif (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_FPU)
#include "FPU/BvFastQuat_FPU.inl"
#endif
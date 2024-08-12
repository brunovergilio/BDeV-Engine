#pragma once


#include "BDeV/Core/Math/BvMathDefs.h"


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_SIMD)
#include "BDeV/Core/Math/SIMD/BvFastVec_SIMD.inl"
#elif (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_FPU)
#include "BDeV/Core/Math/FPU/BvFastVec_FPU.inl"
#endif
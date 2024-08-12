#pragma once


#include "BDeV/Core/Math/BvMathDefs.h"


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_SIMD)
#include "BDeV/Core/Math/SIMD/BvFastQuat_SIMD.inl"
#elif (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_FPU)
#include "BDeV/Core/Math/FPU/BvFastQuat_FPU.inl"
#endif
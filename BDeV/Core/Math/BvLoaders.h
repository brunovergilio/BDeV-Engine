#pragma once


#include "BDeV/Core/Math/BvMathDefs.h"


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_SIMD)
#include "BDeV/Core/Math/SIMD/BvLoaders_SIMD.inl"
#elif (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_FPU)
#include "BDeV/Core/Math/FPU/BvLoaders_FPU.inl"
#endif
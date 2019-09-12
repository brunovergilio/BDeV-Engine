#pragma once


#include "BvCore/BvDefines.h"


#define BV_MATH_TYPE_SIMD 1
#define BV_MATH_TYPE_FPU 2


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#define BV_MATH_USE_TYPE BV_MATH_TYPE_SIMD
#else
#define BV_MATH_USE_TYPE BV_MATH_TYPE_FPU
#endif


#if (BV_MATH_USE_TYPE == BV_MATH_TYPE_SIMD)
//#include <mmintrin.h>  // MMX
//#include <xmmintrin.h> // SSE
//#include <emmintrin.h> // SSE2
//#include <pmmintrin.h> // SSE3
//#include <tmmintrin.h> // SSSE3
//#include <smmintrin.h> // SSE4.1
#include <nmmintrin.h> // SSE4.2
//#include <ammintrin.h> // SSE4A
//#include <wmmintrin.h> // AES
//#include <immintrin.h> // AVX
//#include <zmmintrin.h> // AVX512
#endif

#include <cmath>


// Default floating point error threshold
constexpr float BvEpsilon = 0.0001f;

constexpr float BvPi      = 3.141592653589793238462f;
constexpr float Bv2Pi     = 6.283185482025146484375f;
constexpr float Bv3PiDiv2 = 4.712388992309570312500f;

constexpr float BvPiDiv2  = 1.570796370506286621094f;
constexpr float BvPiDiv3  = 1.047197580337524414063f;
constexpr float BvPiDiv4  = 0.785398163397448309616f;
constexpr float BvPiDiv5  = 0.628318548202514648438f;
constexpr float BvPiDiv6  = 0.523598790168762207031f;
constexpr float BvPiDiv8  = 0.392699092626571655273f;
constexpr float BvPiDiv10 = 0.314159274101257324219f;
constexpr float BvPiDiv12 = 0.261799395084381103516f;
constexpr float BvPiDiv16 = 0.196349546313285827637f;
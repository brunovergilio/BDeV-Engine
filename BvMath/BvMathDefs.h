#pragma once


#define BV_MATH_TYPE_SIMD 1
#define BV_MATH_TYPE_FPU 2


#if defined(_WIN32)
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
//#include <nmmintrin.h> // SSE4.2
//#include <ammintrin.h> // SSE4A
//#include <wmmintrin.h> // AES
#include <immintrin.h> // AVX
//#include <zmmintrin.h> // AVX512
#endif

#include <cmath>


// Default floating point error threshold
constexpr float kEpsilon = 0.0001f;

constexpr float kPi      = 3.141592653589793238462f;
constexpr float k2Pi	 = kPi * 2.0f;//6.283185482025146484375f;
constexpr float k3PiDiv2 = kPi * 1.5f;//4.712388992309570312500f;

constexpr float kPiDiv2  = kPi / 2.0f;//1.570796370506286621094f;
constexpr float kPiDiv3  = kPi / 3.0f;//1.047197580337524414063f;
constexpr float kPiDiv4  = kPi / 4.0f;//0.785398163397448309616f;
constexpr float kPiDiv5  = kPi / 5.0f;//0.628318548202514648438f;
constexpr float kPiDiv6  = kPi / 6.0f;//0.523598790168762207031f;
constexpr float kPiDiv8  = kPi / 8.0f;//0.392699092626571655273f;
constexpr float kPiDiv10 = kPi / 10.0f;//0.314159274101257324219f;
constexpr float kPiDiv12 = kPi / 12.0f;//0.261799395084381103516f;
constexpr float kPiDiv16 = kPi / 16.0f;//0.196349546313285827637f;
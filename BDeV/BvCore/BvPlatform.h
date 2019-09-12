#pragma once


// Definition for the platforms
#define BV_PLATFORM_WIN32 1


// Platform definitions
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#define BV_PLATFORM BV_PLATFORM_WIN32
#else
#error "Platform not yet supported"
#endif


// Platform-dependent stuff
#if (BV_PLATFORM == BV_PLATFORM_WIN32)

#if defined(BV_DLL_EXPORT_DEF)
#define BV_DLL_EXPORT __declspec(dllexport)
#else
#define BV_DLL_EXPORT __declspec(dllimport)
#endif // #if defined(BV_DLL_EXPORT_DEF)

#define BV_ALIGN(size) __declspec(align(size))

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>

#define BvThreadLocal thread_local

#define BvSleep Sleep
#define BvPrint printf
#define BvPause _mm_pause

// Compiler-dependent stuff
#if defined(_MSC_VER)
#define BV_FUNCTION __FUNCTION__
#define BV_INLINE __forceinline
#define BV_CPP_VER _MSVC_LANG

#pragma warning(disable:4201) // nonstandard extension used: nameless struct/union
#pragma warning(disable:4458) // declaration of 'variable' hides class member
#pragma warning(disable:4521) // multiple copy constructors specified
#pragma warning(disable:4324) // structure was padded due to alignment specifier
#pragma warning(disable:4312) // conversion from 'src' to 'dst' of greater size
#pragma warning(disable:4018) // signed/unsigned mismatch
#pragma warning(disable:4311) // pointer truncation from 'src' to 'dst'
#pragma warning(disable:4302) // truncation from 'src' to 'dst'
#pragma warning(disable:4244) // conversion from 'src' to 'dst', possible loss of data
#pragma warning(disable:4267) // conversion from 'src' to 'dst', possible loss of data

#else
#define BV_FUNCTION __func__
#define BV_INLINE inline
#define BV_CPP_VER __cplusplus
#endif // #if defined(_MSC_VER)

#else

// Other platforms

#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)


#if defined(_DEBUG) || defined(DEBUG)
#define BV_DEBUG
#endif // #if defined(_DEBUG) || defined(DEBUG)
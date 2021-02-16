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

#if defined (BV_DLL_EXPORT)
#define BV_API __declspec(dllexport)
#else
#define BV_API __declspec(dllimport)
#endif
#define BV_EXTERN_C extern "C"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#if (_MSC_VER >= 1915)
#define no_init_all deprecated
#endif

#include <Windows.h>

#define BV_PAUSE YieldProcessor()
#define aligned_alloc _aligned_malloc
#define aligned_free _aligned_free

using FileHandle = HANDLE;
using MutexHandle = CRITICAL_SECTION;
using EventHandle = HANDLE;

// Compiler-dependent stuff
#if defined(_MSC_VER)
#define BV_FUNCTION __FUNCTION__
#define BV_FILE __FILE__
#define BV_LINE __LINE__
#define BV_INLINE __forceinline
#define BV_CPP_VER _MSVC_LANG

#pragma warning(disable:4100) // unref
#pragma warning(disable:4189) // init but unref
#pragma warning(disable:4201) // nonstandard extension used: nameless struct/union
#pragma warning(disable:4458) // declaration of 'variable' hides class member
#pragma warning(disable:4521) // multiple copy constructors specified
#pragma warning(disable:4324) // structure was padded due to alignment specifier
#pragma warning(disable:4312) // conversion from 'src' to 'dst' of greater size
#pragma warning(disable:4018) // comparison signed/unsigned mismatch
#pragma warning(disable:4245) // initialization signed/unsigned mismatch
#pragma warning(disable:4311) // pointer truncation from 'src' to 'dst'
#pragma warning(disable:4302) // truncation from 'src' to 'dst'
#pragma warning(disable:4244) // conversion from 'src' to 'dst', possible loss of data
#pragma warning(disable:4267) // conversion from 'src' to 'dst', possible loss of data
#pragma warning(disable:6100) // deref possible nullptr

#else
#define BV_FUNCTION __func__
#define BV_INLINE inline
#define BV_CPP_VER __cplusplus
#endif // #if defined(_MSC_VER)

#else

#error "Platform not yet supported"

#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)


#if defined(_DEBUG) || defined(DEBUG)
#define BV_DEBUG
#endif // #if defined(_DEBUG) || defined(DEBUG)


#include <cstdint>


typedef int8_t    i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float	 f32;
typedef double	 f64;


constexpr u32 kInfinite = UINT32_MAX;
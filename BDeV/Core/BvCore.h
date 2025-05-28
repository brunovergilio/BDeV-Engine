#pragma once


// =================================
// Platform definitions
// =================================
#define BV_PLATFORM_WIN32 1
#define BV_PLATFORM_MACOS 2
#define BV_PLATFORM_LINUX 3

#if defined(_WIN32) || defined(_WIN64)
	#define BV_PLATFORM BV_PLATFORM_WIN32
#elif defined(__APPLE__)
	#define BV_PLATFORM BV_PLATFORM_MACOS
#elif defined(__linux__)
	#define BV_PLATFORM BV_PLATFORM_LINUX
#endif


// =================================
// Compiler definitions
// =================================
#define BV_COMPILER_MSVC 1

#if defined(_MSC_VER)
	#define BV_COMPILER BV_COMPILER_MSVC
#endif


#define BV_MATH_INSTRUCTION_AVX 1
#define BV_MATH_INSTRUCTION_FPU 2


#if defined(__AVX__) || defined(__AVX2__)
#define BV_MATH_INSTRUCTION BV_MATH_INSTRUCTION_AVX
#else
#define BV_MATH_INSTRUCTION BV_MATH_INSTRUCTION_FPU
#endif


// =================================
// Platform-dependent stuff
// =================================
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	#ifndef STRICT
	#define STRICT
	#endif
	
	#ifndef NOMINMAX
	#define NOMINMAX
	#endif
	
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	
	#ifndef VC_EXTRALEAN
	#define VC_EXTRALEAN
	#endif
#else
	#error "Platform not yet supported"
#endif


// =================================
// Compiler-dependent stuff
// =================================
#if (BV_COMPILER == BV_COMPILER_MSVC)
	#define BV_COMPILER_VERSION _MSC_VER
	
	#if (_MSC_VER >= 1915)
		#define no_init_all deprecated
	#endif

	#if !defined(_CRT_SECURE_NO_WARNINGS)
		#define _CRT_SECURE_NO_WARNINGS
	#endif

	#define BV_FUNCTION __FUNCTION__
	#define BV_INLINE __forceinline
	#define BV_NO_INLINE __declspec(noinline)
	
	#pragma warning(disable:4100)	// unref
	#pragma warning(disable:4103)	// alignment changed after including header, may be due to missing #pragma pack(pop)
	#pragma warning(disable:4189)	// init but unref
	#pragma warning(disable:4201)	// nonstandard extension used: nameless struct/union
	#pragma warning(disable:4458)	// declaration of 'variable' hides class member
	#pragma warning(disable:4521)	// multiple copy constructors specified
	#pragma warning(disable:4324)	// structure was padded due to alignment specifier
	#pragma warning(disable:4312)	// conversion from 'src' to 'dst' of greater size
	#pragma warning(disable:4018)	// comparison signed/unsigned mismatch
	#pragma warning(disable:4245)	// initialization signed/unsigned mismatch
	#pragma warning(disable:4311)	// pointer truncation from 'src' to 'dst'
	#pragma warning(disable:4302)	// truncation from 'src' to 'dst'
	#pragma warning(disable:4244)	// conversion from 'src' to 'dst', possible loss of data
	#pragma warning(disable:4267)	// conversion from 'src' to 'dst', possible loss of data
	#pragma warning(disable:6011)	// deref possible nullptr
	#pragma warning(disable:26812)	// unscoped enum
	#pragma warning(disable:4251)	// object needs to have dll-interface to be used by clients of specific class
	#pragma warning(disable:4996)	// function or variable unsafe
	#pragma warning(disable:26495)	// Variable 'variable' is uninitialized. Always initialize a member variable (type.6)
	//#pragma warning(disable:4477)	// format string 'string' requires an argument of type 'type', but variadic argument number has type 'type'
	#pragma warning(disable:6255)	// _alloca indicates failure by raising a stack overflow exception. Consider using _malloca instead
	#pragma warning(disable:4828)	// The file contains a character starting at offset 'offset' that is illegal in the current source character set (codepage 'cp')
	#pragma warning(disable:26110)	// Caller failing to hold lock 'lock' before calling function 'func'

	#define BV_API

#else
	#error "Compiler not yet supported"
#endif


#if defined(_DEBUG) || defined(DEBUG)
	#define BV_DEBUG 1
#else
	#define BV_DEBUG 0
#endif


#include <limits>
#include <cstdint>
#include <new>
#include <type_traits>

using i8	= std::int8_t;
using i16	= std::int16_t;
using i32	= std::int32_t;
using i64	= std::int64_t;

using u8	= std::uint8_t;
using u16	= std::uint16_t;
using u32	= std::uint32_t;
using u64	= std::uint64_t;

using f32	= float;
using f64	= double;

union MemType
{
	void* pAsVoidPtr;
	char* pAsCharPtr;
	size_t* pAsSizeTPtr;
	size_t asSizeT;
};

template<typename Type> constexpr const Type kMin = []() -> auto
{
	if constexpr (std::is_floating_point_v<Type>)
	{
		return std::numeric_limits<Type>::lowest();
	}
	else
	{
		return std::numeric_limits<Type>::min();
	}
}();
template<typename Type> constexpr const Type kMax = std::numeric_limits<Type>::max();

constexpr const auto kI8Min = kMin<i8>;
constexpr const auto kI8Max = kMax<i8>;
constexpr const auto kU8Min = kMin<u8>;
constexpr const auto kU8Max = kMax<u8>;

constexpr const auto kI16Min = kMin<i16>;
constexpr const auto kI16Max = kMax<i16>;
constexpr const auto kU16Min = kMin<u16>;
constexpr const auto kU16Max = kMax<u16>;

constexpr const auto kI32Min = kMin<i32>;
constexpr const auto kI32Max = kMax<i32>;
constexpr const auto kU32Min = kMin<u32>;
constexpr const auto kU32Max = kMax<u32>;

constexpr const auto kI64Min = kMin<i64>;
constexpr const auto kI64Max = kMax<i64>;
constexpr const auto kU64Min = kMin<u64>;
constexpr const auto kU64Max = kMax<u64>;

constexpr const auto kF32Min = kMin<f32>;
constexpr const auto kF32Max = kMax<f32>;
constexpr const auto kF64Min = kMin<f64>;
constexpr const auto kF64Max = kMax<f64>;

template<typename Type>
inline constexpr bool IsPodV = std::is_trivial_v<Type> && std::is_standard_layout_v<Type>;

constexpr size_t kCacheLineSize = std::hardware_destructive_interference_size;

constexpr size_t kPointerSize = sizeof(void*);
constexpr size_t kDefaultAlignmentSize = __STDCPP_DEFAULT_NEW_ALIGNMENT__;


constexpr u64 operator""_kb(size_t bytes)
{
	return bytes * 1024;
}


constexpr u64 operator""_mb(size_t bytes)
{
	return bytes * 1024 * 1024;
}


constexpr u64 operator""_gb(size_t bytes)
{
	return bytes * 1024 * 1024 * 1024;
}


template<typename T, auto V>
constexpr size_t OffsetOfPtr()
{
	return ((size_t)&reinterpret_cast<char const volatile&>((((T*)0)->*V)));
}

#define BV_OFFSETOF(t, v) OffsetOfPtr<t, &t::v>()


struct BvSourceInfo
{
	const char* m_pFunction;
	const char* m_pFile;
	i32 m_Line;
};

#if BV_DEBUG
#define BV_SOURCE_INFO BvSourceInfo{ BV_FUNCTION, __FILE__, __LINE__ }
#else
#define BV_SOURCE_INFO BvSourceInfo{}
#endif


#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#define BV_STACK_ALLOC(size) _alloca(size)
#endif
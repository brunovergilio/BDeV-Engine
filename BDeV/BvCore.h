#pragma once


// Platform definitions
#define BV_PLATFORM_WIN32 1

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
	#define BV_PLATFORM BV_PLATFORM_WIN32
#else
	#error "Platform not yet supported"
#endif

// Compiler definitions
#define BV_COMPILER_MSVC 1

#if defined(_MSC_VER)
	#define BV_COMPILER BV_COMPILER_MSVC
	#define BV_COMPILER_VERSION _MSC_VER
#else
	#error "Compiler not yet supported"
#endif


// Platform-dependent stuff
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
	#define BV_EXTERN_C extern "C"
	
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	
	#if (_MSC_VER >= 1915)
		#define no_init_all deprecated
	#endif

	// Compiler-dependent stuff
	#if BV_COMPILER == BV_COMPILER_MSVC
		#if defined(BV_STATIC_LIB)
			#define BV_API
		#else
			#if defined (BV_DLL_EXPORT)
				#define BV_API __declspec(dllexport)
			#else
				#define BV_API __declspec(dllimport)
			#endif
		#endif

		#if defined(BV_STATIC_LIB)
			#define BV_PLUGIN_API
		#else
			#if defined (BV_PLUGIN_DLL_EXPORT)
				#define BV_PLUGIN_API __declspec(dllexport)
			#else
				#define BV_PLUGIN_API __declspec(dllimport)
			#endif
		#endif

		#define BV_FUNCTION __FUNCTION__
		#define BV_FILE __FILE__
		#define BV_LINE __LINE__
		#define BV_INLINE __forceinline
		#define BV_CPP_VER _MSVC_LANG
		
		#pragma warning(disable:4100) // unref
		#pragma warning(disable:4103) // alignment changed after including header, may be due to missing #pragma pack(pop)
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
		#pragma warning(disable:6011) // deref possible nullptr
		#pragma warning(disable:26812) // unscoped enum
		#pragma warning(disable:4251) // object needs to have dll-interface to be used by clients of specific class
		#pragma warning(disable:4996) // function or variable unsafe
	
	#else
		#if defined(BV_STATIC_LIB)
			#define BV_API
		#else
			#if defined (BV_DLL_EXPORT)
				#define BV_API __attribute__ (dllexport)
			#else
				#define BV_API __attribute__ (dllimport)
			#endif
		#endif

		#if defined(BV_STATIC_LIB)
			#define BV_PLUGIN_API
		#else
			#if defined (BV_PLUGIN_DLL_EXPORT)
				#define BV_PLUGIN_API __attribute__ (dllexport)
			#else
				#define BV_PLUGIN_API __attribute__ (dllimport)
			#endif
		#endif

		#define BV_FUNCTION __func__
		#define BV_FILE __FILE__
		#define BV_LINE __LINE__
		#define BV_INLINE inline
		#define BV_CPP_VER __cplusplus
	#endif
#else
	#error "Platform not yet supported"
#endif


#if defined(_DEBUG) || defined(DEBUG)
	#define BV_DEBUG 1
#else
	#define BV_DEBUG 0
#endif


#include <limits>
#include <cstdint>
#include <new>
#include <algorithm>
#include <type_traits>

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
constexpr size_t kDefaultAlignmentSize = kPointerSize;
constexpr size_t kMinAllocationSize = kPointerSize;


struct BvSourceInfo
{
	const char* m_pFunction;
	const char* m_pFile;
	size_t m_Line;
};


#define BV_SOURCE_INFO BvSourceInfo{ BV_FUNCTION, BV_FILE, BV_LINE }
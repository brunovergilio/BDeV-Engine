#pragma once


#include "BDeV/Core/BvCore.h"


#define BV_NOCOPY(className)					  		\
public:											  		\
	className(const className &) = delete;			  	\
	className & operator =(const className &) = delete; \

#define BV_NOCOPYMOVE(className)					  	\
public:											  		\
	className(const className &) = delete;			  	\
	className & operator =(const className &) = delete; \
	className(className &&) = delete;			  		\
	className & operator =(className &&) = delete;		\

#define BV_DEFAULTCOPYMOVE(className)					\
public:											  		\
	className(const className &) = default;			  	\
	className & operator =(const className &) = default;\
	className(className &&) = default;			  		\
	className & operator =(className &&) = default;		\


#define BvBit(bit) (1 << (bit))


template<u32 N, class Type>
constexpr u32 ArraySize(Type(&)[N])
{
	return N;
}


template<typename Type1, typename Type2>
constexpr std::conditional_t<(sizeof(Type1) >= sizeof(Type2)), Type1, Type2> RoundToNearestPowerOf2(Type1 value, Type2 multiple)
{
	const std::conditional_t<(sizeof(Type1) >= sizeof(Type2)), Type1, Type2> mask = multiple - 1;
	return (value + mask) & (~mask);
}


template<typename Type1, typename Type2>
constexpr std::conditional_t<(sizeof(Type1) >= sizeof(Type2)), Type1, Type2> RoundToNearestMultiple(const Type1 value, const Type2 multiple)
{
	return ((value + multiple - 1) / multiple) * multiple;
}


template<typename Type>
constexpr Type CalculateNewContainerSize(Type value)
{
	return value + (value >> 1) + Type(value <= 2);
}


constexpr size_t ConstexprStringLength(const char* pBytes)
{
	if (*pBytes)
	{
		return 1 + ConstexprStringLength(pBytes + 1);
	}

	return 0;
}


template<class Type>
struct UseEnumClassOperators
{
	static constexpr const bool value = false;
};


template<class Type>
inline constexpr const bool UseEnumClassOperatorsV = UseEnumClassOperators<Type>::value;


#define BV_USE_ENUM_CLASS_OPERATORS(enumClass)				\
template<>													\
struct UseEnumClassOperators<enumClass>						\
{															\
	static constexpr const bool value = true;				\
};


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type operator|(Type v1, Type v2)
{
	return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) |
		static_cast<std::underlying_type_t<Type>>(v2));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type operator|(std::underlying_type_t<Type> v1, Type v2)
{
	return static_cast<Type>(v1 | static_cast<std::underlying_type_t<Type>>(v2));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type operator|(Type v1, std::underlying_type_t<Type> v2)
{
	return static_cast<Type>(v2 | static_cast<std::underlying_type_t<Type>>(v1));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type operator&(Type v1, Type v2)
{
	return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) &
		static_cast<std::underlying_type_t<Type>>(v2));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type operator&(std::underlying_type_t<Type> v1, Type v2)
{
	return static_cast<Type>(v1 & static_cast<std::underlying_type_t<Type>>(v2));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type operator&(Type v1, std::underlying_type_t<Type> v2)
{
	return static_cast<Type>(v2 & static_cast<std::underlying_type_t<Type>>(v1));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type operator^(Type v1, Type v2)
{
	return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) ^
		static_cast<std::underlying_type_t<Type>>(v2));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type operator^(std::underlying_type_t<Type> v1, Type v2)
{
	return static_cast<Type>(v1 ^ static_cast<std::underlying_type_t<Type>>(v2));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type operator^(Type v1, std::underlying_type_t<Type> v2)
{
	return static_cast<Type>(v2 ^ static_cast<std::underlying_type_t<Type>>(v1));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type operator~(Type v1)
{
	return static_cast<Type>(~static_cast<std::underlying_type_t<Type>>(v1));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type& operator|=(Type& v1, Type v2)
{
	v1 = static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) | static_cast<std::underlying_type_t<Type>>(v2));

	return v1;
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type& operator|=(Type& v1, std::underlying_type_t<Type> v2)
{
	v1 = static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) | v2);

	return v1;
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type& operator&=(Type& v1, Type v2)
{
	v1 = static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) & static_cast<std::underlying_type_t<Type>>(v2));

	return v1;
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type& operator&=(Type& v1, std::underlying_type_t<Type> v2)
{
	v1 = static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) & v2);

	return v1;
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type& operator^=(Type& v1, Type v2)
{
	v1 = static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) ^ static_cast<std::underlying_type_t<Type>>(v2));

	return v1;
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE Type& operator^=(Type& v1, std::underlying_type_t<Type> v2)
{
	v1 = static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) ^ v2);

	return v1;
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE bool EHasFlag(Type value, Type flag)
{
	return (value & flag) == flag;
}

template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>&& UseEnumClassOperatorsV<Type>>>
BV_INLINE bool EHasAnyFlags(Type value, Type flags)
{
	return (static_cast<std::underlying_type_t<Type>>(value) & static_cast<std::underlying_type_t<Type>>(flags)) != 0;
}

template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type>>>
BV_INLINE std::underlying_type_t<Type> EVal(Type value)
{
	return static_cast<std::underlying_type_t<Type>>(value);
}


// Taken from http://jhnet.co.uk/articles/cpp_magic
#define EVAL(...) EVAL1024(__VA_ARGS__)
#define EVAL1024(...) EVAL512(EVAL512(__VA_ARGS__))
#define EVAL512(...) EVAL256(EVAL256(__VA_ARGS__))
#define EVAL256(...) EVAL128(EVAL128(__VA_ARGS__))
#define EVAL128(...) EVAL64(EVAL64(__VA_ARGS__))
#define EVAL64(...) EVAL32(EVAL32(__VA_ARGS__))
#define EVAL32(...) EVAL16(EVAL16(__VA_ARGS__))
#define EVAL16(...) EVAL8(EVAL8(__VA_ARGS__))
#define EVAL8(...) EVAL4(EVAL4(__VA_ARGS__))
#define EVAL4(...) EVAL2(EVAL2(__VA_ARGS__))
#define EVAL2(...) EVAL1(EVAL1(__VA_ARGS__))
#define EVAL1(...) __VA_ARGS__

#define FIRST(a, ...) a
#define SECOND(a, b, ...) b

#define IS_PROBE(...) SECOND(__VA_ARGS__, 0)
#define PROBE() ~, 1

#define CAT(a,b) a ## b
#define NOT(x) IS_PROBE(CAT(_NOT_, x))
#define _NOT_0 PROBE()
#define COND(x) NOT(NOT(x))

#define IF_ELSE(condition) _IF_ELSE(COND(condition))
#define _IF_ELSE(condition) CAT(_IF_, condition)

#define _IF_1(...) __VA_ARGS__ _IF_1_ELSE
#define _IF_0(...)             _IF_0_ELSE

#define _IF_1_ELSE(...)
#define _IF_0_ELSE(...) __VA_ARGS__

#define _END_OF_ARGUMENTS_() 0
#define HAS_ARGS(...) COND(FIRST(_END_OF_ARGUMENTS_ __VA_ARGS__)())

#define EMPTY()
#define DEFER1(m) m EMPTY()
#define DEFER2(m) m EMPTY EMPTY()()
#define DEFER3(m) m EMPTY EMPTY EMPTY()()()

//#define FOR_EACH(m, first, ...) m(first)	\
//	IF_ELSE(HAS_ARGS(__VA_ARGS__))			\
//	(										\
//		DEFER2(_FOR_EACH)()(m, __VA_ARGS__)	\
//	)										\
//	(										\
//	)
//#define _FOR_EACH() FOR_EACH
#pragma once


#include "BDeV/System/Debug/BvDebug.h"


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
constexpr std::conditional_t<(sizeof(Type1) > sizeof(Type2)), Type1, Type2> RoundToNearestPowerOf2(Type1 value, Type2 multiple)
{
	BvAssert((multiple & (multiple - 1)) == 0, "Multiple has to be a power of 2");
	const std::conditional_t<(sizeof(Type1) > sizeof(Type2)), Type1, Type2> mask = multiple - 1;
	return (value + mask) & (~mask);
}


template<typename Type1, typename Type2>
constexpr std::conditional_t<(sizeof(Type1) > sizeof(Type2)), Type1, Type2> RoundToNearestMultiple(const Type1 value, const Type2 multiple)
{
	return ((value + multiple - 1) / multiple) * multiple;
}


template<typename Type>
constexpr Type CalculateNewContainerSize(Type value)
{
	return value + (value >> 1) + Type(value <= 2);
}
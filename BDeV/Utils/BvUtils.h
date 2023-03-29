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
constexpr u32 BvArraySize(Type(&)[N])
{
	return N;
}


template<typename Type>
Type RoundToNearestMultipleP2(const Type value, const Type multiple)
{
	BvAssert((multiple & (multiple - 1)) == 0, "Multiple has to be a power of 2");
	const Type mask = multiple - 1;
	return (value + mask) & (~mask);
}


template<typename Type>
Type RoundToNearestMultiple(const Type value, const Type multiple)
{
	return ((value + multiple - 1) / multiple) * multiple;
}


template<typename Type>
Type GetNextPowerOf2(const Type value)
{
	Type result = 0;
	Type count = sizeof(Type) * 8;
	for (Type i = 0; i < count; i++)
	{
		result = static_cast<Type>(1) << i;
		if (result > value)
		{
			return result;
		}
	}

	return 0;
}
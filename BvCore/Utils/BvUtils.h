#pragma once


#include "BvCore/Utils/BvDebug.h"
#include <tuple>


namespace Internal
{
	template<size_t... N>
	struct IndexSequence {};

	// One way to do it (don't like this too much though)
	template<size_t I, size_t... N>
	auto MakeIndexSequence()
	{
		if constexpr (I == 0) return IndexSequence<N...>();
		else return MakeIndexSequence<I - 1, I - 1, N...>();
	}

	// The other way, which I prefer
	template<size_t I, size_t... N>
	struct MakeSequence : public MakeSequence<I - 1, I - 1, N...>
	{
	};

	template<size_t... N>
	struct MakeSequence<0, N...>
	{
		using type = IndexSequence<N...>;
	};

	template<size_t N>
	using MakeSequenceT = typename MakeSequence<N>::type;


	template<typename Fn, typename Tuple, size_t... Indices>
	auto RunHelper(Fn && fn, Tuple && tuple, IndexSequence<Indices...> && indices)
	{
		return fn(std::get<Indices>(tuple)...);
	}


	template<typename Fn, typename... Args>
	auto Run(Fn && fn, Args &&... args)
	{
		return RunHelper(std::forward<Fn>(fn), std::make_tuple<Args...>(std::forward<Args>(args)...), MakeIndexSequence<sizeof...(Args)>());
	}
}


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
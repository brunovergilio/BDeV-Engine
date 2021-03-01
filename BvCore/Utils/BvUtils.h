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
#pragma once


#include "../BvDebug.h"
#include <tuple>


#if defined(BV_DEBUG)


template<class Type>
class BvSingletonValidator
{
public:
	BvSingletonValidator()
	{
		BvAssertMsg(m_Initialized == false, "This class can only have a single instance");
		m_Initialized = true;
	}
	~BvSingletonValidator()
	{
		m_Initialized = false;
	}

private:
	static bool m_Initialized;
};


template<class Type>
bool BvSingletonValidator<Type>::m_Initialized = false;


#define BV_SINGLETON(Class) BvSingletonValidator<Class> m_SingletonValidator;

#else

#define BV_SINGLETON(Class)

#endif // #if defined(BV_DEBUG)


struct NoCopy
{
public:
	NoCopy() = default;
	~NoCopy() = default;

	NoCopy(NoCopy &&) = default;
	NoCopy & operator =(NoCopy &&) = default;

private:
	NoCopy(const NoCopy &) = delete;
	NoCopy & operator =(const NoCopy &) = delete;
};


#define BvDelete(ptr) if (ptr) \
{ \
	delete ptr; \
	ptr = nullptr; \
}

#define BvDeleteArray(ptr) if (ptr) \
{ \
	delete[] ptr; \
	ptr = nullptr; \
}


#define BvBit(bit) (1 << bit)


template<u32 N, class Type>
constexpr u32 BvFixedVectorSize(Type(&)[N])
{
	return N;
}


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


class BvDelegateBase
{
public:
	virtual ~BvDelegateBase() {}
	virtual void Invoke() const = 0;
};


template<class Fn, class... Args>
class BvDelegate : public BvDelegateBase
{
public:
	template<typename = typename std::enable_if<std::is_same<std::invoke_result_t<Fn, Args...>, void>::value>::type>
	BvDelegate(Fn && fn, Args &&... args)
		: m_Function(std::forward<Fn>(fn)), m_Args(std::forward<Args>(args)...) {}

	void Invoke() const override
	{
		// I was initially using the index sequence above and the call helpers, but because I'm using C++ 17, I'll just stick to apply for now
		std::apply(m_Function, m_Args);
	}

private:
	Fn m_Function;
	std::tuple<Args...> m_Args;
};
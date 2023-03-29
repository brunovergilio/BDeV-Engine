#pragma once


#include <tuple>


class BV_API BvDelegateBase
{
public:
	virtual ~BvDelegateBase() {}
	virtual void Invoke() const = 0;
};


template<class Fn, class... Args>
class BV_API BvDelegate final : public BvDelegateBase
{
public:
	template<typename = typename std::enable_if_t<std::is_same_v<std::invoke_result_t<Fn, Args...>, void>&& std::is_invocable_v<Fn, Args...>>>
	BvDelegate(Fn&& fn, Args &&... args)
		: m_Function(std::forward<Fn>(fn)), m_Args(std::forward<Args>(args)...) {}
	~BvDelegate() {}

	void Invoke() const override
	{
		std::apply(m_Function, m_Args);
	}

private:
	Fn m_Function;
	std::tuple<Args...> m_Args;
};


template<size_t JobSize>
class BV_API BvJobT
{
public:
	BvJobT() {}

	template<typename Fn, typename... Args,
		typename = typename std::enable_if_t<
		!std::is_same_v<std::decay_t<Fn>, BvJobT>
		&& std::is_invocable_v<Fn, Args...>
		&& std::is_same_v<std::invoke_result_t<Fn, Args...>, void>>>
	explicit BvJobT(Fn&& fn, Args &&... args)
	{
		BvCompilerAssert(sizeof(BvDelegate<Fn, Args...>) <= JobSize, "Job object is too big");
		Set(std::forward<Fn>(fn), std::forward<Args>(args)...);
	}

	explicit BvJobT(const BvJobT& rhs)
	{
		memcpy(m_Data, rhs.m_Data, JobSize);
	}

	BvJobT& operator=(const BvJobT& rhs)
	{
		if (this != &rhs)
		{
			this->~BvJobT();
			memcpy(m_Data, rhs.m_Data, JobSize);
		}

		return *this;
	}

	explicit BvJobT(BvJobT&& rhs) noexcept
	{
		*this = std::move(rhs);
	}

	BvJobT& operator=(BvJobT&& rhs) noexcept
	{
		if (this != &rhs)
		{
			this->~BvJobT();
			memcpy(m_Data, rhs.m_Data, JobSize);
			rhs.~BvJobT();
		}

		return *this;
	}

	~BvJobT()
	{
		if (IsSet())
		{
			reinterpret_cast<BvDelegateBase* const>(m_Data)->~BvDelegateBase();
			Reset();
		}
	}

	void Run() const
	{
		BvAssert(IsSet(), "Job doesn't have an entry point");
		reinterpret_cast<const BvDelegateBase* const>(m_Data)->Invoke();
	}

	template<typename Fn, typename... Args,
		typename = typename std::enable_if_t<
		!std::is_same_v<std::decay_t<Fn>, BvJobT>
		&& std::is_same_v<std::invoke_result_t<Fn, Args...>, void>>>
	void Set(Fn&& fn, Args &&... args)
	{
		BvCompilerAssert(sizeof(BvDelegate<Fn, Args...>) <= JobSize, "Job object is too big");
		new (m_Data) BvDelegate<Fn, Args...>(std::forward<Fn>(fn), std::forward<Args>(args)...);
	}

	bool IsSet() const
	{
		auto vp = *reinterpret_cast<const u64* const>(m_Data);
		return vp != 0;
	}

	void Reset()
	{
		auto pVp = reinterpret_cast<u64* const>(m_Data);
		pVp = 0;
	}

private:
	u8 m_Data[JobSize]{};
};
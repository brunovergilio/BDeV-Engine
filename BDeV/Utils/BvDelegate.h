#pragma once


#include <tuple>


class BV_API BvDelegateBase
{
public:
	virtual ~BvDelegateBase() {}
	virtual void Invoke() const = 0;
};


template<class Fn, class... Args>
class BvDelegate final : public BvDelegateBase
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
class BvTaskT
{
public:
	BvTaskT() {}

	template<typename Fn, typename... Args,
		typename = typename std::enable_if_t<
		!std::is_same_v<std::decay_t<Fn>, BvTaskT>
		&& std::is_invocable_v<Fn, Args...>
		&& std::is_same_v<std::invoke_result_t<Fn, Args...>, void>>>
	explicit BvTaskT(Fn&& fn, Args &&... args)
	{
		BvCompilerAssert(sizeof(BvDelegate<Fn, Args...>) <= JobSize, "Job object is too big");
		Set(std::forward<Fn>(fn), std::forward<Args>(args)...);
	}

	explicit BvTaskT(const BvTaskT& rhs)
	{
		memcpy(m_Data, rhs.m_Data, JobSize);
	}

	BvTaskT& operator=(const BvTaskT& rhs)
	{
		if (this != &rhs)
		{
			this->~BvTaskT();
			memcpy(m_Data, rhs.m_Data, JobSize);
		}

		return *this;
	}

	explicit BvTaskT(BvTaskT&& rhs) noexcept
	{
		*this = std::move(rhs);
	}

	BvTaskT& operator=(BvTaskT&& rhs) noexcept
	{
		if (this != &rhs)
		{
			this->~BvTaskT();
			memcpy(m_Data, rhs.m_Data, JobSize);
			rhs.~BvTaskT();
		}

		return *this;
	}

	~BvTaskT()
	{
		if (IsSet())
		{
			Reset();
		}
	}

	void Run() const
	{
		BvAssert(IsSet(), "Job doesn't have an entry point");
		m_pDelegate->Invoke();
	}

	template<typename Fn, typename... Args,
		typename = typename std::enable_if_t<
		!std::is_same_v<std::decay_t<Fn>, BvTaskT>
		&& std::is_same_v<std::invoke_result_t<Fn, Args...>, void>>>
	void Set(Fn&& fn, Args &&... args)
	{
		BvCompilerAssert(sizeof(BvDelegate<Fn, Args...>) <= JobSize, "Job object is too big");
		new (m_Data) BvDelegate<Fn, Args...>(std::forward<Fn>(fn), std::forward<Args>(args)...);
	}

	bool IsSet() const
	{
		return m_pDelegate != nullptr;
	}

	void Reset()
	{
		m_pDelegate = nullptr;
	}

private:
	union
	{
		u8 m_Data[JobSize]{};
		BvDelegateBase* m_pDelegate;
	};
};
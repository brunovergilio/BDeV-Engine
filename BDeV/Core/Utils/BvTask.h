#pragma once


#include <tuple>


class IBvTask
{
public:
	virtual ~IBvTask() {}
	virtual void Run() const = 0;
};


namespace Internal
{
	template<class Fn, class... Args>
	class BvTaskT final : public IBvTask
	{
	public:
		template<typename = typename std::enable_if_t<std::is_same_v<std::invoke_result_t<Fn, Args...>, void> && std::is_invocable_v<Fn, Args...>>>
		BvTaskT(Fn&& fn, Args&&... args)
			: m_Function(std::forward<Fn>(fn)), m_Args(std::forward<Args>(args)...) {}
		~BvTaskT() {}

		void Run() const override
		{
			std::apply(m_Function, m_Args);
		}

	private:
		Fn m_Function;
		std::tuple<Args...> m_Args;
	};
}


template<size_t JobSize>
class BvTaskN
{
public:
	BvTaskN() {}

	template<typename Fn, typename... Args,
		typename = typename std::enable_if_t<
		!std::is_same_v<std::decay_t<Fn>, BvTaskN>
		&& std::is_invocable_v<Fn, Args...>
		&& std::is_same_v<std::invoke_result_t<Fn, Args...>, void>>>
	explicit BvTaskN(Fn&& fn, Args&&... args)
	{
		static_assert(sizeof(Internal::BvTaskT<Fn, Args...>) <= JobSize, "Job object is too big, use a bigger task size");
		Set(std::forward<Fn>(fn), std::forward<std::decay_t<Args>>(args)...);
	}

	explicit BvTaskN(const BvTaskN& rhs)
	{
		memcpy(m_Data, rhs.m_Data, JobSize);
	}

	BvTaskN& operator=(const BvTaskN& rhs)
	{
		if (this != &rhs)
		{
			memcpy(m_Data, rhs.m_Data, JobSize);
		}

		return *this;
	}

	explicit BvTaskN(BvTaskN&& rhs) noexcept
	{
		*this = std::move(rhs);
	}

	BvTaskN& operator=(BvTaskN&& rhs) noexcept
	{
		if (this != &rhs)
		{
			memcpy(m_Data, rhs.m_Data, JobSize);
		}

		return *this;
	}

	~BvTaskN()
	{
		Reset();
	}

	void Run() const
	{
		if (IsSet())
		{
			m_pTask->Run();
		}
	}

	template<typename Fn, typename... Args,
		typename = typename std::enable_if_t<
		!std::is_same_v<std::decay_t<Fn>, BvTaskN>
		&& std::is_same_v<std::invoke_result_t<Fn, Args...>, void>>>
	void Set(Fn&& fn, Args&&... args)
	{
		static_assert(sizeof(Internal::BvTaskT<Fn, Args...>) <= JobSize, "Job object is too big");
		new (m_Data) Internal::BvTaskT<Fn, Args...>(std::forward<Fn>(fn), std::forward<std::decay_t<Args>>(args)...);
	}

	bool IsSet() const
	{
		return m_pTask != nullptr;
	}

	void Reset()
	{
		m_pTask = nullptr;
	}

private:
	union
	{
		u8 m_Data[JobSize]{};
		const IBvTask* m_pTask;
	};
};
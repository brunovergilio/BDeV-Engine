#pragma once


#include <tuple>


class IBvTask
{
public:
	virtual ~IBvTask() {}
	virtual void Run() = 0;
	virtual void CloneTo(void* pNewTask) = 0;
	virtual void MoveTo(void* pNewTask) = 0;
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
		BvTaskT(Fn& fn, std::tuple<Args...>& args)
			: m_Function(fn), m_Args(args) {}
		~BvTaskT() {}

		void Run() override
		{
			std::apply(m_Function, m_Args);
		}

		void CloneTo(void* pNewTask) override
		{
			new(pNewTask) BvTaskT<Fn, Args...>(m_Function, m_Args);
		}

		void MoveTo(void* pNewTask) override
		{
			new(pNewTask) BvTaskT<Fn, Args...>(std::forward<Fn>(m_Function), std::forward<Args>(m_Args)...);
		}

	private:
		Fn m_Function;
		std::tuple<Args...> m_Args;
	};
}


template<size_t TaskSize>
class BvTaskN
{
public:
	static_assert(TaskSize >= 16, "Task size must be at least 16 bytes");

	BvTaskN() {}

	template<typename Fn, typename... Args,
		typename = typename std::enable_if_t<!std::is_same_v<std::decay_t<Fn>, BvTaskN>>>
	explicit BvTaskN(Fn&& fn, Args&&... args)
	{
		static_assert(sizeof(Internal::BvTaskT<Fn, Args...>) <= TaskSize, "Task object is too big, use a bigger task size");
		Set(std::forward<Fn>(fn), std::forward<std::decay_t<Args>>(args)...);
	}

	explicit BvTaskN(const BvTaskN& rhs)
	{
		((IBvTask*)rhs.m_Data)->CloneTo(m_Data);
	}

	BvTaskN& operator=(const BvTaskN& rhs)
	{
		if (this != &rhs)
		{
			((IBvTask*)rhs.m_Data)->CloneTo(m_Data);
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
			((IBvTask*)rhs.m_Data)->MoveTo(m_Data);
		}

		return *this;
	}

	~BvTaskN()
	{
		Reset();
	}

	void Run()
	{
		if (IsSet())
		{
			((IBvTask*)m_Data)->Run();
		}
	}

	template<typename Fn, typename... Args,
		typename = typename std::enable_if_t<!std::is_same_v<std::decay_t<Fn>, BvTaskN>>>
	void Set(Fn&& fn, Args&&... args)
	{
		static_assert(sizeof(Internal::BvTaskT<Fn, Args...>) <= TaskSize, "Task object is too big, use a bigger task size");
		new (m_Data) Internal::BvTaskT<Fn, Args...>(std::forward<Fn>(fn), std::forward<std::decay_t<Args>>(args)...);
	}

	bool IsSet() const
	{
		return *((u64*)m_Data) != 0;
	}

	void Reset()
	{
		if (IsSet())
		{
			((IBvTask*)m_Data)->~IBvTask();
			*((u64*)m_Data) = 0;
		}
	}

private:
	u8 m_Data[TaskSize]{};
};
#pragma once


#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
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
	alignas(16) u8 m_Data[TaskSize]{};
};


template<size_t TaskSize>
class BvTask
{
	using TaskFn = void(*)(void*);
	using DtorFn = void(*)(void*);
	using CopyFn = void(*)(void*, const void*);
	using MoveFn = void(*)(void*, void*);

public:
	BvTask() = default;

	~BvTask()
	{
		Reset();
	}

	template<typename Fn, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Fn>, BvTask>>>
	explicit BvTask(Fn&& fn)
	{
		Set(std::forward<Fn>(fn));
	}

	BvTask(const BvTask& other)
	{
		if (other.m_Copy)
		{
			other.m_Copy(m_Data, other.m_Data);
			m_Func = other.m_Func;
			m_Dtor = other.m_Dtor;
			m_Copy = other.m_Copy;
			m_Move = other.m_Move;
		}
	}

	BvTask(BvTask&& other) noexcept
	{
		if (other.m_Move)
		{
			other.m_Move(m_Data, other.m_Data);
			m_Func = other.m_Func;
			m_Dtor = other.m_Dtor;
			m_Copy = other.m_Copy;
			m_Move = other.m_Move;

			other.Reset();
		}
	}

	BvTask& operator=(const BvTask& other)
	{
		if (this != &other)
		{
			Reset();
			if (other.m_Copy)
			{
				other.m_Copy(m_Data, other.m_Data);
				m_Func = other.m_Func;
				m_Dtor = other.m_Dtor;
				m_Copy = other.m_Copy;
				m_Move = other.m_Move;
			}
		}
		return *this;
	}

	BvTask& operator=(BvTask&& other) noexcept
	{
		if (this != &other)
		{
			Reset();
			if (other.m_Move)
			{
				other.m_Move(m_Data, other.m_Data);
				m_Func = other.m_Func;
				m_Dtor = other.m_Dtor;
				m_Copy = other.m_Copy;
				m_Move = other.m_Move;

				other.Reset();
			}
		}
		return *this;
	}

	template<typename Fn>
	void Set(Fn&& fn)
	{
		using FnType = std::decay_t<Fn>;

		static_assert(sizeof(FnType) <= TaskSize, "Function object is too big, use a bigger task size");

		Reset();

		new(m_Data) FnType(std::forward<Fn>(fn));
		m_Func = [](void* data)
			{
				(*reinterpret_cast<FnType*>(data))();
			};

		m_Dtor = [](void* data)
			{
				reinterpret_cast<FnType*>(data)->~FnType();
			};

		m_Copy = [](void* dst, const void* src)
			{
				new(dst) FnType(*reinterpret_cast<const FnType*>(src));
			};

		m_Move = [](void* dst, void* src)
			{
				new(dst) FnType(std::move(*reinterpret_cast<FnType*>(src)));
			};
	}

	void Reset()
	{
		if (m_Dtor)
		{
			m_Dtor(m_Data);
		}

		m_Func = nullptr;
		m_Dtor = nullptr;
		m_Copy = nullptr;
		m_Move = nullptr;
	}

	void operator()()
	{
		BV_ASSERT(m_Func != nullptr, "Task needs a function");
		m_Func(m_Data);
	}

	explicit operator bool() const
	{
		return m_Func != nullptr;
	}

private:
	u8 m_Data[TaskSize]{};
	TaskFn m_Func = nullptr;
	DtorFn m_Dtor = nullptr;
	CopyFn m_Copy = nullptr;
	MoveFn m_Move = nullptr;
};
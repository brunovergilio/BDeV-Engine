#pragma once


#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"
#include "BvUtils.h"


// ==========================================================================
// Basic task objects, which allows setting up lambdas or function pointers.
// ==========================================================================


namespace Internal
{
	template<bool Move, bool Copy>
	struct BvTaskData
	{
	};
	
	template<>
	struct BvTaskData<false, false>
	{
		using TaskFn = void(*)(void*);
		using DtorFn = void(*)(void*);

		TaskFn m_Func = nullptr;
		DtorFn m_Dtor = nullptr;
	};

	template<>
	struct BvTaskData<true, false> : public BvTaskData<false, false>
	{
		using MoveFn = void(*)(void*, void*);

		MoveFn m_Move = nullptr;
	};

	template<>
	struct BvTaskData<true, true> : public BvTaskData<true, false>
	{
		using CopyFn = void(*)(void*, const void*);

		CopyFn m_Copy = nullptr;
	};
}


// Task object without copy / move functionality
template<size_t TaskSize>
class BvTask : public Internal::BvTaskData<false, false>
{
	BV_NOCOPYMOVE(BvTask);

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
	}

	void Reset()
	{
		if (m_Dtor)
		{
			m_Dtor(m_Data);
		}

		m_Func = nullptr;
		m_Dtor = nullptr;
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
	alignas(8) u8 m_Data[TaskSize]{};
};



// Task object with move (but not copy) functionality
template<size_t TaskSize>
class BvMTask : public Internal::BvTaskData<true, false>
{
	BV_NOCOPY(BvMTask);

public:
	BvMTask() = default;

	BvMTask(BvMTask&& other) noexcept
	{
		if (other.m_Move)
		{
			other.m_Move(m_Data, other.m_Data);
			m_Func = other.m_Func;
			m_Dtor = other.m_Dtor;
			m_Move = other.m_Move;
		}
	}

	BvMTask& operator=(BvMTask&& other) noexcept
	{
		if (this != &other)
		{
			if (other.m_Move)
			{
				Reset();
				other.m_Move(m_Data, other.m_Data);
				m_Func = other.m_Func;
				m_Dtor = other.m_Dtor;
				m_Move = other.m_Move;
			}
		}
		return *this;
	}

	~BvMTask()
	{
		Reset();
	}

	template<typename Fn, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Fn>, BvMTask>>>
	explicit BvMTask(Fn&& fn)
	{
		Set(std::forward<Fn>(fn));
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
	alignas(8) u8 m_Data[TaskSize]{};
};


// Task object with copy / move functionality
template<size_t TaskSize>
class BvCMTask : public Internal::BvTaskData<true, true>
{
public:
	BvCMTask() = default;

	~BvCMTask()
	{
		Reset();
	}

	template<typename Fn, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Fn>, BvCMTask>>>
	explicit BvCMTask(Fn&& fn)
	{
		Set(std::forward<Fn>(fn));
	}

	BvCMTask(const BvCMTask& other)
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

	BvCMTask(BvCMTask&& other) noexcept
	{
		if (other.m_Move)
		{
			other.m_Move(m_Data, other.m_Data);
			m_Func = other.m_Func;
			m_Dtor = other.m_Dtor;
			m_Copy = other.m_Copy;
			m_Move = other.m_Move;
		}
	}

	BvCMTask& operator=(const BvCMTask& other)
	{
		if (this != &other)
		{
			if (other.m_Copy)
			{
				Reset();
				other.m_Copy(m_Data, other.m_Data);
				m_Func = other.m_Func;
				m_Dtor = other.m_Dtor;
				m_Copy = other.m_Copy;
				m_Move = other.m_Move;
			}
		}
		return *this;
	}

	BvCMTask& operator=(BvCMTask&& other) noexcept
	{
		if (this != &other)
		{
			if (other.m_Move)
			{
				Reset();
				other.m_Move(m_Data, other.m_Data);
				m_Func = other.m_Func;
				m_Dtor = other.m_Dtor;
				m_Copy = other.m_Copy;
				m_Move = other.m_Move;
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
	alignas(8) u8 m_Data[TaskSize]{};
};
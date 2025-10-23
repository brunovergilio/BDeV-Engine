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
		m_Func = [](void* pData)
			{
				(*reinterpret_cast<FnType*>(pData))();
			};

		m_Dtor = [](void* pData)
			{
				reinterpret_cast<FnType*>(pData)->~FnType();
			};
	}

	void Reset()
	{
		if (m_Dtor)
		{
			m_Dtor(m_Data);

			m_Func = nullptr;
			m_Dtor = nullptr;
		}
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

	BvMTask(BvMTask&& rhs) noexcept
	{
		if (rhs.m_Move)
		{
			rhs.m_Move(m_Data, rhs.m_Data);
			m_Func = rhs.m_Func;
			m_Dtor = rhs.m_Dtor;
			m_Move = rhs.m_Move;

			rhs.m_Func = nullptr;
			rhs.m_Dtor = nullptr;
			rhs.m_Move = nullptr;
		}
	}

	BvMTask& operator=(BvMTask&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Reset();

			if (rhs.m_Move)
			{
				rhs.m_Move(m_Data, rhs.m_Data);
				m_Func = rhs.m_Func;
				m_Dtor = rhs.m_Dtor;
				m_Move = rhs.m_Move;

				rhs.m_Func = nullptr;
				rhs.m_Dtor = nullptr;
				rhs.m_Move = nullptr;
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
		m_Func = [](void* pData)
			{
				(*reinterpret_cast<FnType*>(pData))();
			};

		m_Dtor = [](void* pData)
			{
				reinterpret_cast<FnType*>(pData)->~FnType();
			};

		m_Move = [](void* pDst, void* pSrc)
			{
				new(pDst) FnType(std::move(*reinterpret_cast<FnType*>(pSrc)));
			};
	}

	void Reset()
	{
		if (m_Dtor)
		{
			m_Dtor(m_Data);

			m_Func = nullptr;
			m_Dtor = nullptr;
			m_Move = nullptr;
		}
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

	BvCMTask(const BvCMTask& rhs)
	{
		if (rhs.m_Copy)
		{
			rhs.m_Copy(m_Data, rhs.m_Data);
			m_Func = rhs.m_Func;
			m_Dtor = rhs.m_Dtor;
			m_Copy = rhs.m_Copy;
			m_Move = rhs.m_Move;
		}
	}

	BvCMTask(BvCMTask&& rhs) noexcept
	{
		if (rhs.m_Move)
		{
			rhs.m_Move(m_Data, rhs.m_Data);
			m_Func = rhs.m_Func;
			m_Dtor = rhs.m_Dtor;
			m_Copy = rhs.m_Copy;
			m_Move = rhs.m_Move;

			rhs.m_Func = nullptr;
			rhs.m_Dtor = nullptr;
			rhs.m_Copy = nullptr;
			rhs.m_Move = nullptr;
		}
	}

	BvCMTask& operator=(const BvCMTask& rhs)
	{
		if (this != &rhs)
		{
			Reset();

			if (rhs.m_Copy)
			{
				rhs.m_Copy(m_Data, rhs.m_Data);
				m_Func = rhs.m_Func;
				m_Dtor = rhs.m_Dtor;
				m_Copy = rhs.m_Copy;
				m_Move = rhs.m_Move;
			}
		}
		return *this;
	}

	BvCMTask& operator=(BvCMTask&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Reset();

			if (rhs.m_Move)
			{
				rhs.m_Move(m_Data, rhs.m_Data);
				m_Func = rhs.m_Func;
				m_Dtor = rhs.m_Dtor;
				m_Copy = rhs.m_Copy;
				m_Move = rhs.m_Move;

				rhs.m_Func = nullptr;
				rhs.m_Dtor = nullptr;
				rhs.m_Copy = nullptr;
				rhs.m_Move = nullptr;
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
		m_Func = [](void* pData)
			{
				(*reinterpret_cast<FnType*>(pData))();
			};

		m_Dtor = [](void* pData)
			{
				reinterpret_cast<FnType*>(pData)->~FnType();
			};

		m_Copy = [](void* pDst, const void* pSrc)
			{
				new(pDst) FnType(*reinterpret_cast<const FnType*>(pSrc));
			};

		m_Move = [](void* pDst, void* pSrc)
			{
				new(pDst) FnType(std::move(*reinterpret_cast<FnType*>(pSrc)));
			};
	}

	void Reset()
	{
		if (m_Dtor)
		{
			m_Dtor(m_Data);

			m_Func = nullptr;
			m_Dtor = nullptr;
			m_Copy = nullptr;
			m_Move = nullptr;
		}
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
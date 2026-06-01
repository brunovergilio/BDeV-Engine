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

	template<bool Move, bool Copy, typename... Args>
	struct BvTaskDataArgs
	{
	};

	template<typename... Args>
	struct BvTaskDataArgs<false, false, Args...>
	{
		using TaskFn = void(*)(void*, Args...);
		using DtorFn = void(*)(void*);

		TaskFn m_Func = nullptr;
		DtorFn m_Dtor = nullptr;
	};

	template<typename... Args>
	struct BvTaskDataArgs<true, false, Args...> : public BvTaskDataArgs<false, false, Args...>
	{
		using MoveFn = void(*)(void*, void*);

		MoveFn m_Move = nullptr;
	};

	template<typename... Args>
	struct BvTaskDataArgs<true, true, Args...> : public BvTaskDataArgs<true, false, Args...>
	{
		using CopyFn = void(*)(void*, const void*);

		CopyFn m_Copy = nullptr;
	};

	template<typename Fn, typename... Bound>
	struct BoundCallable
	{
		using FnT = std::decay_t<Fn>;
		using TupleT = std::tuple<std::decay_t<Bound>...>;

		FnT fn;
		TupleT args;

		template<typename F, typename... B>
		BoundCallable(F&& f, B&&... b)
			: fn(std::forward<F>(f))
			, args(std::forward<B>(b)...)
		{
		}

		void operator()()
		{
			std::apply(fn, args);
		}
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
	alignas(kPointerSize) u8 m_Data[TaskSize]{};
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
	alignas(kPointerSize) u8 m_Data[TaskSize]{};
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
	alignas(kPointerSize) u8 m_Data[TaskSize]{};
};


// Non-copyable, non-movable task that accepts parameters
template<size_t TaskSize, typename... Args>
class BvTaskArgs : public Internal::BvTaskDataArgs<false, false, Args...>
{
	BV_NOCOPYMOVE(BvTaskArgs);

public:
	BvTaskArgs() = default;
	~BvTaskArgs() { Reset(); }

	template<typename Fn, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Fn>, BvTaskArgs>>>
	explicit BvTaskArgs(Fn&& fn) { Set(std::forward<Fn>(fn)); }

	template<typename Fn>
	void Set(Fn&& fn)
	{
		using FnType = std::decay_t<Fn>;
		static_assert(sizeof(FnType) <= TaskSize, "Function object is too big, use a bigger task size");

		Reset();

		new(m_Data) FnType(std::forward<Fn>(fn));
		this->m_Func = [](void* pData, Args... args)
			{
				(*reinterpret_cast<FnType*>(pData))(std::forward<Args>(args)...);
			};

		this->m_Dtor = [](void* pData)
			{
				reinterpret_cast<FnType*>(pData)->~FnType();
			};
	}

	void Reset()
	{
		if (this->m_Dtor)
		{
			this->m_Dtor(m_Data);
			this->m_Func = nullptr;
			this->m_Dtor = nullptr;
		}
	}

	void operator()(Args... args)
	{
		BV_ASSERT(this->m_Func != nullptr, "Task needs a function");
		this->m_Func(m_Data, std::forward<Args>(args)...);
	}

	explicit operator bool() const { return this->m_Func != nullptr; }

private:
	alignas(kPointerSize) u8 m_Data[TaskSize]{};
};

// Move-only version
template<size_t TaskSize, typename... Args>
class BvMTaskArgs : public Internal::BvTaskDataArgs<true, false, Args...>
{
	BV_NOCOPY(BvMTaskArgs);

public:
	BvMTaskArgs() = default;

	BvMTaskArgs(BvMTaskArgs&& rhs) noexcept
	{
		if (rhs.m_Move)
		{
			rhs.m_Move(m_Data, rhs.m_Data);
			this->m_Func = rhs.m_Func;
			this->m_Dtor = rhs.m_Dtor;
			this->m_Move = rhs.m_Move;

			rhs.m_Func = nullptr;
			rhs.m_Dtor = nullptr;
			rhs.m_Move = nullptr;
		}
	}

	BvMTaskArgs& operator=(BvMTaskArgs&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Reset();
			if (rhs.m_Move)
			{
				rhs.m_Move(m_Data, rhs.m_Data);
				this->m_Func = rhs.m_Func;
				this->m_Dtor = rhs.m_Dtor;
				this->m_Move = rhs.m_Move;

				rhs.m_Func = nullptr;
				rhs.m_Dtor = nullptr;
				rhs.m_Move = nullptr;
			}
		}
		return *this;
	}

	~BvMTaskArgs() { Reset(); }

	template<typename Fn, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Fn>, BvMTaskArgs>>>
	explicit BvMTaskArgs(Fn&& fn) { Set(std::forward<Fn>(fn)); }

	template<typename Fn>
	void Set(Fn&& fn)
	{
		using FnType = std::decay_t<Fn>;
		static_assert(sizeof(FnType) <= TaskSize, "Function object is too big, use a bigger task size");

		Reset();

		new(m_Data) FnType(std::forward<Fn>(fn));
		this->m_Func = [](void* pData, Args... args)
			{
				(*reinterpret_cast<FnType*>(pData))(std::forward<Args>(args)...);
			};

		this->m_Dtor = [](void* pData)
			{
				reinterpret_cast<FnType*>(pData)->~FnType();
			};

		this->m_Move = [](void* pDst, void* pSrc)
			{
				new(pDst) FnType(std::move(*reinterpret_cast<FnType*>(pSrc)));
			};
	}

	void Reset()
	{
		if (this->m_Dtor)
		{
			this->m_Dtor(m_Data);
			this->m_Func = nullptr;
			this->m_Dtor = nullptr;
			this->m_Move = nullptr;
		}
	}

	void operator()(Args... args)
	{
		BV_ASSERT(this->m_Func != nullptr, "Task needs a function");
		this->m_Func(m_Data, std::forward<Args>(args)...);
	}

	explicit operator bool() const { return this->m_Func != nullptr; }

private:
	alignas(kPointerSize) u8 m_Data[TaskSize]{};
};

// Copy + move version
template<size_t TaskSize, typename... Args>
class BvCMTaskArgs : public Internal::BvTaskDataArgs<true, true, Args...>
{
public:
	BvCMTaskArgs() = default;
	~BvCMTaskArgs() { Reset(); }

	template<typename Fn, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Fn>, BvCMTaskArgs>>>
	explicit BvCMTaskArgs(Fn&& fn) { Set(std::forward<Fn>(fn)); }

	BvCMTaskArgs(const BvCMTaskArgs& rhs)
	{
		if (rhs.m_Copy)
		{
			rhs.m_Copy(m_Data, rhs.m_Data);
			this->m_Func = rhs.m_Func;
			this->m_Dtor = rhs.m_Dtor;
			this->m_Copy = rhs.m_Copy;
			this->m_Move = rhs.m_Move;
		}
	}

	BvCMTaskArgs(BvCMTaskArgs&& rhs) noexcept
	{
		if (rhs.m_Move)
		{
			rhs.m_Move(m_Data, rhs.m_Data);
			this->m_Func = rhs.m_Func;
			this->m_Dtor = rhs.m_Dtor;
			this->m_Copy = rhs.m_Copy;
			this->m_Move = rhs.m_Move;

			rhs.m_Func = nullptr;
			rhs.m_Dtor = nullptr;
			rhs.m_Copy = nullptr;
			rhs.m_Move = nullptr;
		}
	}

	BvCMTaskArgs& operator=(const BvCMTaskArgs& rhs)
	{
		if (this != &rhs)
		{
			Reset();
			if (rhs.m_Copy)
			{
				rhs.m_Copy(m_Data, rhs.m_Data);
				this->m_Func = rhs.m_Func;
				this->m_Dtor = rhs.m_Dtor;
				this->m_Copy = rhs.m_Copy;
				this->m_Move = rhs.m_Move;
			}
		}
		return *this;
	}

	BvCMTaskArgs& operator=(BvCMTaskArgs&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Reset();
			if (rhs.m_Move)
			{
				rhs.m_Move(m_Data, rhs.m_Data);
				this->m_Func = rhs.m_Func;
				this->m_Dtor = rhs.m_Dtor;
				this->m_Copy = rhs.m_Copy;
				this->m_Move = rhs.m_Move;

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
		this->m_Func = [](void* pData, Args... args)
			{
				(*reinterpret_cast<FnType*>(pData))(std::forward<Args>(args)...);
			};

		this->m_Dtor = [](void* pData)
			{
				reinterpret_cast<FnType*>(pData)->~FnType();
			};

		this->m_Copy = [](void* pDst, const void* pSrc)
			{
				new(pDst) FnType(*reinterpret_cast<const FnType*>(pSrc));
			};

		this->m_Move = [](void* pDst, void* pSrc)
			{
				new(pDst) FnType(std::move(*reinterpret_cast<FnType*>(pSrc)));
			};
	}

	void Reset()
	{
		if (this->m_Dtor)
		{
			this->m_Dtor(m_Data);
			this->m_Func = nullptr;
			this->m_Dtor = nullptr;
			this->m_Copy = nullptr;
			this->m_Move = nullptr;
		}
	}

	void operator()(Args... args)
	{
		BV_ASSERT(this->m_Func != nullptr, "Task needs a function");
		this->m_Func(m_Data, std::forward<Args>(args)...);
	}

	explicit operator bool() const { return this->m_Func != nullptr; }

private:
	alignas(kPointerSize) u8 m_Data[TaskSize]{};
};


// Non-copyable, non-movable bound task
template<size_t TaskSize>
class BvTaskBound : public Internal::BvTaskData<false, false>
{
	BV_NOCOPYMOVE(BvTaskBound);

public:
	BvTaskBound() = default;
	~BvTaskBound() { Reset(); }

	// Set a callable and the parameters to bind
	template<typename Fn, typename... Bound,
		typename WCandidate = Internal::BoundCallable<Fn, Bound...>>
		void SetBound(Fn&& fn, Bound&&... bound)
	{
		using WC = WCandidate;
		static_assert(sizeof(WC) <= TaskSize, "Bound callable is too big, use a bigger task size");

		Reset();
		new(m_Data) WC(std::forward<Fn>(fn), std::forward<Bound>(bound)...);

		m_Func = [](void* pData)
			{
				(*reinterpret_cast<WC*>(pData))();
			};

		m_Dtor = [](void* pData)
			{
				reinterpret_cast<WC*>(pData)->~WC();
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

	explicit operator bool() const { return m_Func != nullptr; }

private:
	alignas(kPointerSize) u8 m_Data[TaskSize]{};
};

// Move-only bound task
template<size_t TaskSize>
class BvMTaskBound : public Internal::BvTaskData<true, false>
{
	BV_NOCOPY(BvMTaskBound);

public:
	BvMTaskBound() = default;

	BvMTaskBound(BvMTaskBound&& rhs) noexcept
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

	BvMTaskBound& operator=(BvMTaskBound&& rhs) noexcept
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

	~BvMTaskBound() { Reset(); }

	template<typename Fn, typename... Bound,
		typename WCandidate = Internal::BoundCallable<Fn, Bound...>>
		void SetBound(Fn&& fn, Bound&&... bound)
	{
		using WC = WCandidate;
		static_assert(sizeof(WC) <= TaskSize, "Bound callable is too big, use a bigger task size");

		Reset();
		new(m_Data) WC(std::forward<Fn>(fn), std::forward<Bound>(bound)...);

		m_Func = [](void* pData)
			{
				(*reinterpret_cast<WC*>(pData))();
			};

		m_Dtor = [](void* pData)
			{
				reinterpret_cast<WC*>(pData)->~WC();
			};

		m_Move = [](void* pDst, void* pSrc)
			{
				new(pDst) WC(std::move(*reinterpret_cast<WC*>(pSrc)));
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

	explicit operator bool() const { return m_Func != nullptr; }

private:
	alignas(kPointerSize) u8 m_Data[TaskSize]{};
};

// Copy + move bound task
template<size_t TaskSize>
class BvCMTaskBound : public Internal::BvTaskData<true, true>
{
public:
	BvCMTaskBound() = default;
	~BvCMTaskBound() { Reset(); }

	template<typename Fn, typename... Bound,
		typename WCandidate = Internal::BoundCallable<Fn, Bound...>>
		void SetBound(Fn&& fn, Bound&&... bound)
	{
		using WC = WCandidate;
		static_assert(sizeof(WC) <= TaskSize, "Bound callable is too big, use a bigger task size");

		Reset();
		new(m_Data) WC(std::forward<Fn>(fn), std::forward<Bound>(bound)...);

		m_Func = [](void* pData)
			{
				(*reinterpret_cast<WC*>(pData))();
			};

		m_Dtor = [](void* pData)
			{
				reinterpret_cast<WC*>(pData)->~WC();
			};

		m_Copy = [](void* pDst, const void* pSrc)
			{
				new(pDst) WC(*reinterpret_cast<const WC*>(pSrc));
			};

		m_Move = [](void* pDst, void* pSrc)
			{
				new(pDst) WC(std::move(*reinterpret_cast<WC*>(pSrc)));
			};
	}

	BvCMTaskBound(const BvCMTaskBound& rhs)
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

	BvCMTaskBound(BvCMTaskBound&& rhs) noexcept
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

	BvCMTaskBound& operator=(const BvCMTaskBound& rhs)
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

	BvCMTaskBound& operator=(BvCMTaskBound&& rhs) noexcept
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

	explicit operator bool() const { return m_Func != nullptr; }

private:
	alignas(kPointerSize) u8 m_Data[TaskSize]{};
};
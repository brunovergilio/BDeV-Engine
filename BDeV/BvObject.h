#pragma once

#include <atomic>
#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"


// Managed pointer for the IBvObject interface, heavily based of Microsoft's ComPtr class
template<typename T>
class BvPtr
{
public:
	BvPtr() noexcept
		: m_pObj(nullptr)
	{
	}

	BvPtr(std::nullptr_t) noexcept
		: m_pObj(nullptr)
	{
	}

	BvPtr(T* pObj) noexcept
		: m_pObj(pObj)
	{
		AddRefInternal();
	}

	BvPtr(const BvPtr& rhs) noexcept
		: m_pObj(rhs.m_pObj)
	{
		AddRefInternal();
	}

	BvPtr(BvPtr&& rhs) noexcept
		: m_pObj(nullptr)
	{
		if (this != &rhs)
		{
			Swap(rhs);
		}
	}

	BvPtr& operator=(std::nullptr_t) noexcept
	{
		ReleaseInternal();
		return *this;
	}

	BvPtr& operator=(T* pObj) noexcept
	{
		if (m_pObj != pObj)
		{
			BvPtr(pObj).Swap(*this);
		}
		return *this;
	}

	BvPtr& operator=(const BvPtr& rhs) noexcept
	{
		if (m_pObj != rhs.m_pObj)
		{
			BvPtr(rhs).Swap(*this);
		}
		return *this;
	}

	BvPtr& operator=(BvPtr&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Swap(rhs);
		}

		return *this;
	}

	~BvPtr() noexcept
	{
		ReleaseInternal();
	}

	void Swap(BvPtr& rhs) noexcept
	{
		std::swap(m_pObj, rhs.m_pObj);
	}

	void Swap(BvPtr&& rhs) noexcept
	{
		std::swap(m_pObj, rhs.m_pObj);
	}

	operator bool() const noexcept
	{
		return m_pObj != nullptr;
	}

	T* Get() const noexcept
	{
		return m_pObj;
	}

	T* operator->() const noexcept
	{
		return m_pObj;
	}

	T* Detach() noexcept
	{
		T* ptr = m_pObj;
		m_pObj = nullptr;
		return ptr;
	}

	void Attach(T* pObj) noexcept
	{
		if (m_pObj != nullptr)
		{
			auto ref = m_pObj->Release();
			BvAssert(ref != 0 || m_pObj != pObj, "Can't attach the same pointer when its reference count is 1!");
			if (ref == 0 && m_pObj == pObj)
			{
				m_pObj = nullptr;
				return;
			}
		}

		m_pObj = pObj;
	}

	void Reset()
	{
		ReleaseInternal();
	}

protected:
	void AddRefInternal() const noexcept
	{
		if (m_pObj != nullptr)
		{
			m_pObj->AddRef();
		}
	}

	void ReleaseInternal() noexcept
	{
		u32 ref = 0;
		T* pTemp = m_pObj;

		if (pTemp != nullptr)
		{
			m_pObj = nullptr;
			ref = pTemp->Release();
		}
	}

private:
	T* m_pObj = nullptr;
};
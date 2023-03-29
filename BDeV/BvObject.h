#pragma once

#include <atomic>
#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"


// Interface that serves as a base class for dynamic-only objects
class IBvObjectBase
{
	BV_NOCOPYMOVE(IBvObjectBase);

public:
	IBvObjectBase() {}
	virtual ~IBvObjectBase() {}

	template<typename Type>
	bool As(Type** ppObject) const
	{
		if constexpr (std::is_same_v<std::remove_pointer_t<decltype(this)>, Type>)
		{
			*ppObject = reinterpret_cast<Type*>(this);
			AddRef();
			return true;
		}
		else
		{
			*ppObject = nullptr;
			return false;
		}
	}

	virtual u32 AddRef() = 0;
	virtual u32 Release() = 0;

private:
	std::atomic<u32> m_RefCount = 0;
};
#pragma once


#include "BDeV/Core/Utils/BvHash.h"


template<typename T>
struct TypeInfo
{
	static BV_INLINE u64 GetId()
	{
		static u8 id;
		return reinterpret_cast<u64>(&id);
	}

	static BV_INLINE const char* GetName()
	{
		#if BV_PLATFORM_WIN32
			constexpr u32 kFunctionFrontSize = sizeof("TypeInfo<") - 1u;
			constexpr u32 kFunctionBackSize = sizeof(">::GetName") - 1u;
		#else
		#endif

#if BV_DEBUG
		constexpr size_t kSize = sizeof(BV_FUNCTION) - kFunctionFrontSize - kFunctionBackSize;
		static const char* typeName = [kSize]()
			{
				static char name[kSize]{};
				memcpy(name, BV_FUNCTION + kFunctionFrontSize, kSize - 1u);
				return name;
			}();

		return typeName;
#else
		return "";
#endif
	}

	static BV_INLINE u64 GetHash()
	{
		static u64 hash = [pName = TypeInfo<T>::GetName()]()
			{
				return MurmurHash64A(pName, strlen(pName));
			}();

		return hash;
	}
};


// TODO: This should be with reflection code
#define BV_ROBJECT(...)
#define BV_RMETHOD(...)
#define BV_RVAR(...)
#define BV_RENUM(...)
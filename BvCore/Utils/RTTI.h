#pragma once


#include "BvCore/BvCore.h"


namespace Internal
{
	template<typename Type>
	BV_INLINE u64 GetTypeId()
	{
		static i32 id;
		return reinterpret_cast<u64>(&id);
	}
}

template<typename Type>
BV_INLINE u64 GetTypeId()
{
	using BaseType = typename std::remove_cv_t<typename std::remove_reference_t<Type>>;
	return Internal::GetTypeId<BaseType>();
}


#define BV_RTTI(className)							\
public:												\
static u64 GetTypeId()								\
{													\
	return GetTypeId<className>();					\
}													\
static constexpr const char * const GetTypeName()	\
{													\
	return #className;								\
}													\
private:											\
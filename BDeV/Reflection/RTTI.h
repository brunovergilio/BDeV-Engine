#pragma once


#include "BDeV/BvCore.h"


namespace Internal
{
	template<typename Type>
	BV_INLINE u64 GetTypeId()
	{
		static u8 id;
		return reinterpret_cast<u64>(&id);
	}
}


template<typename Type>
BV_INLINE u64 GetTypeId()
{
	using BaseType = typename std::remove_cv_t<typename std::remove_reference_t<Type>>;
	return Internal::GetTypeId<BaseType>();
}


//#define BV_RTTI(className)							\
//public:												\
//static u64 GetTypeId()								\
//{													\
//	return GetTypeId<className>();					\
//}													\
//static constexpr const char * const GetTypeName()	\
//{													\
//	return #className;								\
//}													\
//private:											\


#define BV_RCLASS(...)
#define BV_RSTRUCT(...)
#define BV_REFLECT(className, ...)
#define BV_RVAR(...)
#define BV_RMETHOD(...)
#define BV_RENUM(...)
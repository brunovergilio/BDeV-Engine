#pragma once


#include <type_traits>


template<class Type>
struct UseEnumClassOperators
{
	static constexpr bool value = false;
};


#define BV_USE_ENUM_CLASS_OPERATORS(enumClass)				\
typedef std::underlying_type_t<enumClass> enumClass##T;		\
template<>													\
struct UseEnumClassOperators<enumClass>						\
{															\
	static constexpr bool value = true;						\
};


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> operator |(const Type v1, const Type v2)
{
	return static_cast<std::underlying_type_t<Type>>(v1) |
		static_cast<std::underlying_type_t<Type>>(v2);
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> operator |(const std::underlying_type_t<Type> v1, const Type v2)
{
	return v1 | static_cast<std::underlying_type_t<Type>>(v2);
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> operator |(const Type v1, const std::underlying_type_t<Type> v2)
{
	return v2 | static_cast<std::underlying_type_t<Type>>(v1);
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> operator &(const Type v1, const Type v2)
{
	return static_cast<std::underlying_type_t<Type>>(v1) &
		static_cast<std::underlying_type_t<Type>>(v2);
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> operator &(const std::underlying_type_t<Type> v1, const Type v2)
{
	return v1 & static_cast<std::underlying_type_t<Type>>(v2);
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> operator &(const Type v1, const std::underlying_type_t<Type> v2)
{
	return v2 & static_cast<std::underlying_type_t<Type>>(v1);
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> operator ^(const Type v1, const Type v2)
{
	return static_cast<std::underlying_type_t<Type>>(v1) ^
		static_cast<std::underlying_type_t<Type>>(v2);
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> operator ^(const std::underlying_type_t<Type> v1, const Type v2)
{
	return v1 ^ static_cast<std::underlying_type_t<Type>>(v2);
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> operator ^(const Type v1, const std::underlying_type_t<Type> v2)
{
	return v2 ^ static_cast<std::underlying_type_t<Type>>(v1);
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> operator ~(const Type v1)
{
	return ~static_cast<std::underlying_type_t<Type>>(v1);
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> & operator |=(std::underlying_type_t<Type> & v1, const Type v2)
{
	v1 |= static_cast<std::underlying_type_t<Type>>(v2);

	return v1;
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> & operator &=(std::underlying_type_t<Type> & v1, const Type v2)
{
	v1 &= static_cast<std::underlying_type_t<Type>>(v2);

	return v1;
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE std::underlying_type_t<Type> & operator ^=(std::underlying_type_t<Type> & v1, const Type v2)
{
	v1 ^= static_cast<std::underlying_type_t<Type>>(v2);

	return v1;
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
constexpr std::underlying_type_t<Type> EnumVal(const Type v)
{
	return static_cast<std::underlying_type_t<Type>>(v);
}
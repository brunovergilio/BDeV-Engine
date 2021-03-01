#pragma once


#include <type_traits>


template<class Type>
struct UseEnumClassOperators
{
	static constexpr const bool value = false;
};


template<class Type>
constexpr const bool UseEnumClassOperatorsV = UseEnumClassOperators<Type>::value;


#define BV_USE_ENUM_CLASS_OPERATORS(enumClass)				\
template<>													\
struct UseEnumClassOperators<enumClass>						\
{															\
	static constexpr const bool value = true;				\
};


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperatorsV<Type>>>
BV_INLINE Type operator |(const Type v1, const Type v2)
{
	return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) |
		static_cast<std::underlying_type_t<Type>>(v2));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE Type operator |(const std::underlying_type_t<Type> v1, const Type v2)
{
	return static_cast<Type>(v1 | static_cast<std::underlying_type_t<Type>>(v2));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE Type operator |(const Type v1, const std::underlying_type_t<Type> v2)
{
	return static_cast<Type>(v2 | static_cast<std::underlying_type_t<Type>>(v1));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE Type operator &(const Type v1, const Type v2)
{
	return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) &
		static_cast<std::underlying_type_t<Type>>(v2));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE Type operator &(const std::underlying_type_t<Type> v1, const Type v2)
{
	return static_cast<Type>(v1 & static_cast<std::underlying_type_t<Type>>(v2));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE Type operator &(const Type v1, const std::underlying_type_t<Type> v2)
{
	return static_cast<Type>(v2 & static_cast<std::underlying_type_t<Type>>(v1));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE Type operator ^(const Type v1, const Type v2)
{
	return static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) ^
		static_cast<std::underlying_type_t<Type>>(v2));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE Type operator ^(const std::underlying_type_t<Type> v1, const Type v2)
{
	return static_cast<Type>(v1 ^ static_cast<std::underlying_type_t<Type>>(v2));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE Type operator ^(const Type v1, const std::underlying_type_t<Type> v2)
{
	return static_cast<Type>(v2 ^ static_cast<std::underlying_type_t<Type>>(v1));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE Type operator ~(const Type v1)
{
	return static_cast<Type>(~static_cast<std::underlying_type_t<Type>>(v1));
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE Type & operator |=(Type & v1, Type v2)
{
	v1 = static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) | static_cast<std::underlying_type_t<Type>>(v2));

	return v1;
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE Type & operator &=(Type & v1, const Type v2)
{
	v1 = static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) & static_cast<std::underlying_type_t<Type>>(v2));

	return v1;
}


template<class Type, typename = typename std::enable_if_t<std::is_enum_v<Type> && UseEnumClassOperators<Type>::value>>
BV_INLINE Type & operator ^=(std::underlying_type_t<Type> & v1, const Type v2)
{
	v1 = static_cast<Type>(static_cast<std::underlying_type_t<Type>>(v1) ^ static_cast<std::underlying_type_t<Type>>(v2));

	return v1;
}
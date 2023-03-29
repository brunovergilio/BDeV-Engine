#pragma once


#include "BDeV/Utils/BvUtils.h"


enum class Protection : u8
{
	kPublic,
	kProtected,
	kPrivate
};


class BvMethodInfo
{
	BV_NOCOPYMOVE(BvMethodInfo);

public:
	constexpr BvMethodInfo() {}
	constexpr BvMethodInfo(const char* const pName, const bool isVirtual, const bool isFinal, const Protection protection)
		: m_pName(pName), m_IsVirtual(isVirtual), m_IsFinal(isFinal), m_Protection(protection) {}

	constexpr const char* const GetName() const { return m_pName; }
	constexpr bool IsVirtual() const { return m_IsVirtual; }
	constexpr bool IsFinal() const { return m_IsFinal; }
	constexpr bool IsOverridable() const { return !m_IsFinal; }
	constexpr Protection GetProtection() const { return m_Protection; }

private:
	const char* const m_pName = nullptr;
	const bool m_IsVirtual = false;
	const bool m_IsFinal = false;
	const Protection m_Protection = Protection::kPublic;
};


class BvClassInfo
{
	BV_NOCOPYMOVE(BvClassInfo);

public:
	constexpr BvClassInfo() {}
	constexpr BvClassInfo(const char* const pName, const BvMethodInfo* pMethods, const u32 methodCount, const bool isFinal)
		: m_pName(pName), m_pMethods(pMethods), m_MethodCount(methodCount), m_IsFinal(isFinal) {}

	constexpr const char* const GetName() const { return m_pName; }
	constexpr bool IsFinal() const { return m_IsFinal; }
	constexpr bool IsOverridable() const { return !m_IsFinal; }

private:
	const char* const m_pName = nullptr;
	const BvMethodInfo* const m_pMethods = nullptr;
	const u32 m_MethodCount = 0;
	const bool m_IsFinal = false;
};


template<typename Type>
struct BvStructClassReflectionData
{
};
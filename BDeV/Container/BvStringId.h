#pragma once


#include "BDeV/BvCore.h"
#include "BvString.h"


class BvStringId
{
public:
	constexpr BvStringId() {}
	constexpr BvStringId(u64 id) : m_Id(id) {}

	BvStringId(const char* pId);
	BvStringId(const BvStringId& rhs);
	BvStringId(const BvString& rhs);
	BvStringId(BvStringId&& rhs) noexcept;

	BvStringId& operator=(const BvStringId& rhs);
	BvStringId& operator=(BvStringId&& rhs) noexcept;
	BvStringId& operator=(const char* pId);
	BvStringId& operator=(u64 id);
	BvStringId& operator=(const BvString& rhs);

	constexpr bool operator==(const BvStringId& rhs) const { return m_Id == rhs.m_Id; }
	constexpr bool operator!=(const BvStringId& rhs) const { return m_Id != rhs.m_Id; }
	constexpr bool operator==(u64 id) const { return m_Id == id; }
	constexpr bool operator!=(u64 id) const { return m_Id != id; }

	bool operator==(const char* pId) const;
	bool operator!=(const char* pId) const;
	bool operator==(const BvString& rhs) const;
	bool operator!=(const BvString& rhs) const;

	const BvString* GetString();

	constexpr operator u64() const { return m_Id; }
	constexpr u64 GetId() const { return m_Id; }

private:
	u64 m_Id{};
};


constexpr u64 operator""_sid(const char* pStr, size_t length)
{
	return BvStringId(Internal::ConstexprMurmurHash64AHelper(pStr, length));
}
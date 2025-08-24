#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvHash.h"


class BvStringId
{
public:
	constexpr BvStringId() : m_Id(Empty()) {}
	constexpr BvStringId(const BvStringId& rhs) : m_Id(rhs.m_Id) {}
	constexpr BvStringId(BvStringId&& rhs) noexcept { m_Id = rhs.m_Id; }
	constexpr BvStringId(u64 id) : m_Id(id) {}
	BvStringId(const char* pId);

	BvStringId& operator=(const BvStringId& rhs);
	BvStringId& operator=(BvStringId&& rhs) noexcept;
	BvStringId& operator=(const char* pId);
	BvStringId& operator=(u64 id);

	constexpr bool operator==(const BvStringId& rhs) const { return m_Id == rhs.m_Id; }
	constexpr bool operator!=(const BvStringId& rhs) const { return m_Id != rhs.m_Id; }
	constexpr bool operator==(u64 id) const { return m_Id == id; }
	constexpr bool operator!=(u64 id) const { return m_Id != id; }

	bool operator==(const char* pId) const;
	bool operator!=(const char* pId) const;

	const char* GetString() const;

	constexpr operator u64() const { return m_Id; }
	constexpr u64 GetId() const { return m_Id; }

	static constexpr BvStringId Empty()
	{
		constexpr auto kEmptyStringId = Internal::ConstexprMurmurHash64AHelper("", 1);
		return kEmptyStringId;
	}

private:
	u64 m_Id;
};


constexpr BvStringId operator""_sid(const char* pStr, size_t length)
{
	return BvStringId(Internal::ConstexprMurmurHash64AHelper(pStr, length));
}


template<>
struct std::hash<BvStringId>
{
	BV_INLINE u64 operator()(const BvStringId& id) const
	{
		return id;
	}
};


#if BV_DEBUG
#define BV_NAME_ID(id) BvStringId(id)
#else
#define BV_NAME_ID(id) BvStringId(id##_sid)
#endif
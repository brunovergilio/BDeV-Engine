#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Utils/BvHash.h"
#include <string_view>


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

	constexpr operator u64() const { return m_Id; }
	constexpr u64 GetId() const { return m_Id; }

	constexpr operator bool() const { return m_Id == Empty().GetId(); }

	static constexpr BvStringId Empty()
	{
		constexpr auto kEmptyStringId = constexpr_xxh3::XXH3_64bits_const("", 0);
		return kEmptyStringId;
	}

private:
	u64 m_Id;
};



namespace Internal
{
	template <size_t N>
	struct LiteralFixedString
	{
		char data[N]{};

		constexpr LiteralFixedString(const char(&str)[N])
		{
			for (std::size_t i = 0; i < N; ++i)
			{
				data[i] = str[i];
			}
		}

		constexpr std::string_view view() const
		{
			return { data, N - 1 };
		}
	};
}


template <Internal::LiteralFixedString S>
constexpr auto operator""_sid()
{
	return BvStringId(constexpr_xxh3::XXH3_64bits_const(S.view()));
}


template<>
struct std::hash<BvStringId>
{
	BV_INLINE u64 operator()(const BvStringId& id) const
	{
		return id;
	}
};


template<typename Hasher>
struct BvHash<BvStringId, Hasher>
{
	BV_INLINE u64 operator()(const BvStringId& id) const
	{
		return id;
	}
};
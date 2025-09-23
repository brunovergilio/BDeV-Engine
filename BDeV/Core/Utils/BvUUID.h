#pragma once


#include "BDeV/Core/BvCore.h"
#include "BvRandom.h"


struct BvUUID
{
	union
	{
		struct
		{
			u32 m_Data1;
			u16 m_Data2;
			u16 m_Data3;
			u16 m_Data4;
			u8 m_Data5[6];
		};
		struct
		{
			u64 m_ULL1;
			u64 m_ULL2;
		};
	};

	friend constexpr bool operator==(const BvUUID& lhs, const BvUUID& rhs)
	{
		return lhs.m_ULL1 == rhs.m_ULL1 && lhs.m_ULL2 == rhs.m_ULL2
			/*&& lhs.m_Data3 == rhs.m_Data3 && lhs.m_Data4 == rhs.m_Data4
			&& lhs.m_Data5[0] == rhs.m_Data5[0] && lhs.m_Data5[1] == rhs.m_Data5[1]
			&& lhs.m_Data5[2] == rhs.m_Data5[2] && lhs.m_Data5[3] == rhs.m_Data5[3]
			&& lhs.m_Data5[4] == rhs.m_Data5[4] && lhs.m_Data5[5] == rhs.m_Data5[5]*/;
	}

	friend constexpr bool operator!=(const BvUUID& lhs, const BvUUID& rhs)
	{
		return !(lhs == rhs);
	}

	BV_INLINE void New()
	{
		BvSplitMix64 gen;
		u64 blocks[] = { gen.Next(), gen.Next() };
		// Set version (4 bits for version, which is 4)
		blocks[0] = (blocks[0] & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
		// Set variant (2 bits for variant, most significant bits are 10)
		blocks[1] = (blocks[1] & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

		m_ULL1 = blocks[0];
		m_ULL2 = blocks[1];
	}
};


namespace Internal::UUID
{
	constexpr bool IsHyphen(const char ch)
	{
		return ch == '-';
	}


	constexpr bool IsValidUUID(const char* pUUID)
	{
		if (!*pUUID)
		{
			return true;
		}

		if ((*pUUID >= '0' && *pUUID <= '9') || (*pUUID >= 'a' && *pUUID <= 'f') || (*pUUID >= 'A' && *pUUID <= 'F') || *pUUID == '-')
		{
			return IsValidUUID(pUUID + 1);
		}

		return false;
	}

	template<typename Type>
	constexpr Type GetValueFromChar(char ch)
	{
		if (ch >= '0' && ch <= '9')
		{
			return (ch - '0');
		}
		if (ch >= 'a' && ch <= 'f')
		{
			return (10) + (ch - 'a');
		}
		if (ch >= 'A' && ch <= 'F')
		{
			return (10) + (ch - 'A');
		}

		return (0);
	}

	template<typename T>
	constexpr T MakeUUIDValueHelper(const char* pStr, size_t charCount)
	{
		if (charCount == 0 || !*pStr || *pStr == '-')
		{
			return 0;
		}

		return (GetValueFromChar<T>(*pStr) << (charCount * 4 - 4)) | MakeUUIDValueHelper<T>(pStr + 1, charCount - 1);
	}

	template<typename T>
	constexpr T MakeUUIDValue(const char* pStr)
	{
		return MakeUUIDValueHelper<T>(pStr, sizeof(T) * 2);
	}
}


template<size_t Size>
constexpr BvUUID MakeUUIDv4(const char(&pUUID)[Size])
{
	if (Size != 37 || !Internal::UUID::IsHyphen(pUUID[8]) || !Internal::UUID::IsHyphen(pUUID[13])
		|| !Internal::UUID::IsHyphen(pUUID[18]) || !Internal::UUID::IsHyphen(pUUID[23]) || !Internal::UUID::IsValidUUID(pUUID))
	{
		return BvUUID();
	}
	
	return BvUUID{ Internal::UUID::MakeUUIDValue<u32>(pUUID), Internal::UUID::MakeUUIDValue<u16>(pUUID + 9),
		Internal::UUID::MakeUUIDValue<u16>(pUUID + 14), Internal::UUID::MakeUUIDValue<u16>(pUUID + 19),
		{ Internal::UUID::MakeUUIDValue<u8>(pUUID + 24), Internal::UUID::MakeUUIDValue<u8>(pUUID + 26),
		Internal::UUID::MakeUUIDValue<u8>(pUUID + 28), Internal::UUID::MakeUUIDValue<u8>(pUUID + 30),
		Internal::UUID::MakeUUIDValue<u8>(pUUID + 32), Internal::UUID::MakeUUIDValue<u8>(pUUID + 34) } };
}
#pragma once


#include "BDeV/Core/BvCore.h"
#include <random>


// UUID Version 4
// Version 4: This is set by ensuring bits 12–15 of data3 are 0100 (for Version 4).
// Variant 1: This is set by ensuring the two highest bits of data4 are 10.
//struct BvGUID
//{
//	inline void Generate()
//	{
//		std::random_device rd;
//		std::mt19937 gen(rd());
//		std::uniform_int_distribution<u32> dist32(0, 0xFFFFFFFF);
//		std::uniform_int_distribution<u16> dist16(0, 0xFFFF);
//		std::uniform_int_distribution<u8> dist8(0, 0xFF);
//
//		m_Data1 = dist32(gen);
//		m_Data2 = dist16(gen);
//		m_Data3 = (dist16(gen) & 0x0FFF) | (4 << 12);  // Set Version 4
//		m_Data4 = (dist16(gen) & 0x3FFF) | 0x8000;     // Set Variant 1
//
//		// Fill data5 with random bytes
//		for (int i = 0; i < 6; ++i)
//		{
//			m_Data5[i] = dist8(gen);
//		}
//	}
//
//	u32 m_Data1;
//	u16 m_Data2;
//	u16 m_Data3;
//	u16 m_Data4;
//	u8 m_Data5[6];
//};


struct BvUUID
{
	u64 m_Block1{};
	u64 m_Block2{};

	//constexpr BvUUID() {}
	//constexpr BvUUID(u64 block1, u64 block2) : m_Block1(block1), m_Block2(block2) {}
	//BvUUID(const BvUUID& rhs) noexcept : m_Block1(rhs.m_Block1), m_Block2(rhs.m_Block2) {}
	//BvUUID& operator=(const BvUUID& rhs) noexcept { m_Block1 = rhs.m_Block1; m_Block2 = rhs.m_Block2; return *this; }
	//BvUUID(BvUUID&& rhs) noexcept : m_Block1(rhs.m_Block1), m_Block2(rhs.m_Block2) {}
	//BvUUID& operator=(BvUUID&& rhs) noexcept { m_Block1 = rhs.m_Block1; m_Block2 = rhs.m_Block2; return *this; }

	friend constexpr bool operator==(const BvUUID& lhs, const BvUUID& rhs)
	{
		return lhs.m_Block1 == rhs.m_Block1 && lhs.m_Block2 == rhs.m_Block2;
	}

	friend constexpr bool operator!=(const BvUUID& lhs, const BvUUID& rhs)
	{
		return !(lhs == rhs);
	}
};


BvUUID GenerateUUIDv4();


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

	constexpr u64 GetValueFromChar(char ch)
	{
		if (ch >= '0' && ch <= '9')
		{
			return u64(ch - '0');
		}
		if (ch >= 'a' && ch <= 'f')
		{
			return 10ull + u64(ch - 'a');
		}
		if (ch >= 'A' && ch <= 'F')
		{
			return 10ull + u64(ch - 'A');
		}

		return 0;
	}

	constexpr u64 MakeUUIDValueHelper(const char* pStr, size_t shift)
	{
		if (!*pStr || *pStr == '-')
		{
			return 0;
		}

		return (GetValueFromChar(*pStr) << (shift - 4)) | MakeUUIDValueHelper(pStr + 1, shift - 4);
	}

	constexpr u64 MakeUUIDv4Part1(const char* pStr)
	{
		return MakeUUIDValueHelper(pStr, 64) | MakeUUIDValueHelper(pStr + 9, 32) | MakeUUIDValueHelper(pStr + 14, 16);
	}

	constexpr u64 MakeUUIDv4Part2(const char* pStr)
	{
		return MakeUUIDValueHelper(pStr + 19, 64) | MakeUUIDValueHelper(pStr + 24, 48);
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

	return BvUUID{ Internal::UUID::MakeUUIDv4Part1(pUUID), Internal::UUID::MakeUUIDv4Part2(pUUID) };
}


struct BvUUIDString
{
	static constexpr auto kUUIDStringSize = 37;
	char m_Value[kUUIDStringSize];
};


constexpr BvUUIDString GetUUIDString(const BvUUID& uuid)
{
	constexpr auto getCharFn = [](u64 block, u64 shift)
		{
			constexpr u8 chs[] =
			{
				'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
			};
			return chs[(block >> shift) & 0xF];
		};

	BvUUIDString str{};
	size_t strIndex = 0;
	for (auto i = 0u; i < 8; ++i)
	{
		str.m_Value[strIndex++] = getCharFn(uuid.m_Block1, 60 - i * 8);
		str.m_Value[strIndex++] = getCharFn(uuid.m_Block1, 56 - i * 8);
		if (strIndex == 8 || strIndex == 13)
		{
			strIndex++;
		}
	}
	for (auto i = 0u; i < 8; ++i)
	{
		if (strIndex == 18 || strIndex == 23)
		{
			strIndex++;
		}
		str.m_Value[strIndex++] = getCharFn(uuid.m_Block2, 60 - i * 8);
		str.m_Value[strIndex++] = getCharFn(uuid.m_Block2, 56 - i * 8);
	}

	str.m_Value[8] = str.m_Value[13] = str.m_Value[18] = str.m_Value[23] = '-';

	return str;
}
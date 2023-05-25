#include "BvStringId.h"
#include "BvRobinMap.h"


#if BV_DEBUG

class BvDebugStringIdTable
{
	BV_NOCOPYMOVE(BvDebugStringIdTable);

public:
	BvDebugStringIdTable() {}
	~BvDebugStringIdTable() {}

	const BvString* AddString(const char* pStr)
	{
		return AddString(pStr, strlen(pStr));
	}

	const BvString* AddString(const char* pStr, u32 length)
	{
		auto hash = MurmurHash64A(pStr, length);
		auto result = m_Table.Emplace(hash, BvString(pStr));
		if (!result.second)
		{
			BV_ERROR("Existing String Id found [%llu]:\ncurrent: %s\nnew:%s", hash, result.first->second.CStr(), pStr);
		}

		return &result.first->second;
	}

private:
	BvRobinMap<u64, BvString> m_Table;
} g_StringIdTable;

#endif


BvStringId::BvStringId(const char* pId)
{
#if BV_DEBUG
	m_pString = g_StringIdTable.AddString(pId);
#endif
	m_Id = MurmurHash64A(pId, strlen(pId));
}


BvStringId::BvStringId(const char* pId, u32 length)
{
#if BV_DEBUG
	m_pString = g_StringIdTable.AddString(pId, length);
#endif
	m_Id = MurmurHash64A(pId, length);
}


BvStringId::BvStringId(const BvStringId& rhs)
	: m_Id(rhs.m_Id)
#if BV_DEBUG
	, m_pString(rhs.m_pString)
#endif
{
}


BvStringId::BvStringId(BvStringId&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvStringId::BvStringId(const BvString& rhs)
	: m_Id(rhs.Hash())
#if BV_DEBUG
	, m_pString(g_StringIdTable.AddString(rhs.CStr(), rhs.Size()))
#endif
{
}


BvStringId& BvStringId::operator=(const BvStringId& rhs)
{
	if (this != &rhs)
	{
		m_Id = rhs.m_Id;
#if BV_DEBUG
		m_pString = rhs.m_pString;
#endif
	}

	return *this;
}


BvStringId& BvStringId::operator=(BvStringId&& rhs) noexcept
{
	if (this != &rhs)
	{
		m_Id = rhs.m_Id;
#if BV_DEBUG
		m_pString = rhs.m_pString;
#endif
	}

	return *this;
}


BvStringId& BvStringId::operator=(const char* pId)
{
#if BV_DEBUG
	m_pString = g_StringIdTable.AddString(pId);
#endif
	m_Id = MurmurHash64A(pId, strlen(pId));

	return *this;
}


BvStringId& BvStringId::operator=(u64 id)
{
	m_Id = id;

	return *this;
}


BvStringId& BvStringId::operator=(const BvString& rhs)
{
#if BV_DEBUG
	m_pString = g_StringIdTable.AddString(rhs.CStr(), rhs.Size());
#endif
	m_Id = rhs.Hash();

	return *this;
}


bool BvStringId::operator==(const char* pId) const
{
	return m_Id == MurmurHash64A(pId, strlen(pId));
}


bool BvStringId::operator!=(const char* pId) const
{
	return !(*this == pId);
}


bool BvStringId::operator==(const BvString& rhs) const
{
	return m_Id == rhs.Hash();
}


bool BvStringId::operator!=(const BvString& rhs) const
{
	return !(*this == rhs);
}


const BvString* BvStringId::GetString()
{
#if BV_DEBUG
	return m_pString;
#else
	return nullptr;
#endif
}
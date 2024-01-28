#include "BvStringId.h"
#include "BvRobinMap.h"



class BvDebugStringIdTable
{
	BV_NOCOPYMOVE(BvDebugStringIdTable);

public:
	BvDebugStringIdTable() {}
	~BvDebugStringIdTable() {}

#if BV_DEBUG
	void AddString(u64 hash, const BvString& str)
	{
		auto result = m_Table.Emplace(hash, BvString(str));
		BvAssert(result.second || (!result.second && result.first->second == str), "Existing String Id found [%llu]:\ncurrent: %s\nnew:%s", hash, result.first->second.CStr(), str.CStr());
	}

	void AddString(u64 hash, const char* pStr, u32 length)
	{
		auto result = m_Table.Emplace(hash, BvString(pStr, 0, length));
		BvAssert(result.second || (!result.second && result.first->second == pStr), "Existing String Id found [%llu]:\ncurrent: %s\nnew:%s", hash, result.first->second.CStr(), pStr);
	}

	const BvString* GetString(u64 id) const
	{
		BvAssert(m_Table.HasKey(id), "String Id not in table");

		return &m_Table.At(id);
	}

private:
	BvRobinMap<u64, BvString> m_Table;
#else
	void AddString(u64, const BvString&) {}
	void AddString(u64, const char*, u32) {}
	const BvString* GetString(u64) const { return nullptr; }
#endif
} g_StringIdTable;



BvStringId::BvStringId(const char* pId)
{
	u32 len = static_cast<u32>(strlen(pId));
	m_Id = MurmurHash64A(pId, len);
	g_StringIdTable.AddString(m_Id, pId, strlen(pId));
}


BvStringId::BvStringId(const BvStringId& rhs)
	: m_Id(rhs.m_Id)
{
}


BvStringId::BvStringId(const BvString& rhs)
	: m_Id(rhs.Hash())
{
	g_StringIdTable.AddString(m_Id, rhs);
}


BvStringId::BvStringId(BvStringId&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvStringId& BvStringId::operator=(const BvStringId& rhs)
{
	if (this != &rhs)
	{
		m_Id = rhs.m_Id;
	}

	return *this;
}


BvStringId& BvStringId::operator=(BvStringId&& rhs) noexcept
{
	if (this != &rhs)
	{
		m_Id = rhs.m_Id;
	}

	return *this;
}


BvStringId& BvStringId::operator=(const char* pId)
{
	u32 len = static_cast<u32>(strlen(pId));
	m_Id = MurmurHash64A(pId, len);
	g_StringIdTable.AddString(m_Id, pId, strlen(pId));

	return *this;
}


BvStringId& BvStringId::operator=(u64 id)
{
	m_Id = id;

	return *this;
}


BvStringId& BvStringId::operator=(const BvString& rhs)
{
	m_Id = rhs.Hash();
	g_StringIdTable.AddString(m_Id, rhs);

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
	return g_StringIdTable.GetString(m_Id);
}
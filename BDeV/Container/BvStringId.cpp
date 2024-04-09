#include "BvStringId.h"
#include "BvRobinMap.h"



class BvDebugStringIdTable
{
	BV_NOCOPYMOVE(BvDebugStringIdTable);

public:
	BvDebugStringIdTable() {}
	~BvDebugStringIdTable() {}

	static BvDebugStringIdTable& GetInstance()
	{
		static BvDebugStringIdTable stringTable;
		return stringTable;
	}

	const BvString& GetEmptyString() const
	{
		static BvString emptyString;
		return emptyString;
	}

#if BV_DEBUG
	void AddString(u64 hash, const BvString& str)
	{
		auto result = m_Table.Emplace(hash, BvString(str));
		//BvAssert(result.second || (!result.second && result.first->second == str), "Existing String Id found [%llu]:\ncurrent: %s\nnew:%s", hash, result.first->second.CStr(), str.CStr());
	}

	void AddString(u64 hash, const char* pStr, u32 length)
	{
		auto result = m_Table.Emplace(hash, BvString(pStr, 0, length));
		//BvAssert(result.second || (!result.second && result.first->second == pStr), "Existing String Id found [%llu]:\ncurrent: %s\nnew:%s", hash, result.first->second.CStr(), pStr);
	}

	const BvString& GetString(u64 id) const
	{
		auto result = m_Table.FindKey(id);
		return result != m_Table.cend() ? result->second : GetEmptyString();
	}

private:
	BvRobinMap<u64, BvString> m_Table;
#else
	void AddString(u64, const BvString&) {}
	void AddString(u64, const char*, u32) {}
	const BvString& GetString(u64) const { return GetEmptyString(); }
#endif
};


BvStringId::BvStringId(const char* pId)
{
	BvAssert(pId != nullptr, "String can't be nullptr");

	u32 len = static_cast<u32>(strlen(pId));
	m_Id = MurmurHash64A(pId, len);
	BvDebugStringIdTable::GetInstance().AddString(m_Id, pId, strlen(pId));
}


BvStringId::BvStringId(const BvStringId& rhs)
	: m_Id(rhs.m_Id)
{
}


BvStringId::BvStringId(const BvString& rhs)
	: m_Id(rhs.Hash())
{
	BvAssert(rhs.CStr() != nullptr, "String can't be nullptr");
	BvDebugStringIdTable::GetInstance().AddString(m_Id, rhs);
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
	BvAssert(pId != nullptr, "String can't be nullptr");

	u32 len = static_cast<u32>(strlen(pId));
	m_Id = MurmurHash64A(pId, len);
	BvDebugStringIdTable::GetInstance().AddString(m_Id, pId, strlen(pId));

	return *this;
}


BvStringId& BvStringId::operator=(u64 id)
{
	m_Id = id;

	return *this;
}


BvStringId& BvStringId::operator=(const BvString& rhs)
{
	BvAssert(rhs.CStr() != nullptr, "String can't be nullptr");

	m_Id = rhs.Hash();
	BvDebugStringIdTable::GetInstance().AddString(m_Id, rhs);

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


const BvString& BvStringId::GetString() const
{
	return BvDebugStringIdTable::GetInstance().GetString(m_Id);
}
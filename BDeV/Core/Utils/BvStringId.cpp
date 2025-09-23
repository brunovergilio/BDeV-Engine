#include "BvStringId.h"
#include "BDeV/Core/Container/BvRobinMap.h"
#include "BDeV/Core/Container/BvString.h"



class BvDebugStringIdTable
{
	BV_NOCOPYMOVE(BvDebugStringIdTable);

	static constexpr const char* kpEmptyString = "";

public:
	BvDebugStringIdTable() {}
	~BvDebugStringIdTable() {}

	static BvDebugStringIdTable& GetInstance()
	{
		static BvDebugStringIdTable stringTable;
		return stringTable;
	}

	const char* GetEmptyString() const
	{
		return kpEmptyString;
	}

#if BV_DEBUG
	void AddString(u64 hash, const char* pStr, u32 length)
	{
		auto it = m_Table.FindKey(hash);
		if (it == m_Table.cend())
		{
			m_Table[hash] = BvString(pStr, 0, length);
		}
		else
		{
			BV_ASSERT(it->second == pStr, "Existing String Id found with hash [%llu]: %s", hash, pStr);
		}
	}

	const char* GetString(u64 id) const
	{
		auto result = m_Table.FindKey(id);
		return result != m_Table.cend() ? result->second.CStr() : GetEmptyString();
	}

private:
	BvRobinMap<u64, BvString> m_Table;
#else
	void AddString(u64, const char*, u32) {}
	const char* GetString(u64) const { return GetEmptyString(); }
#endif
};


BvStringId::BvStringId(const char* pId)
{
	if (!pId)
	{
		*this = Empty();
		return;
	}

	u32 len = static_cast<u32>(std::char_traits<char>::length(pId));
	m_Id = MurmurHash64A(pId, len);
	BvDebugStringIdTable::GetInstance().AddString(m_Id, pId, len);
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
	if (!pId)
	{
		*this = Empty();
		return *this;
	}

	u32 len = static_cast<u32>(std::char_traits<char>::length(pId));
	m_Id = MurmurHash64A(pId, len);
	BvDebugStringIdTable::GetInstance().AddString(m_Id, pId, len);

	return *this;
}


BvStringId& BvStringId::operator=(u64 id)
{
	m_Id = id;

	return *this;
}


bool BvStringId::operator==(const char* pId) const
{
	return m_Id == MurmurHash64A(pId, std::char_traits<char>::length(pId));
}


bool BvStringId::operator!=(const char* pId) const
{
	return !(*this == pId);
}


const char* BvStringId::GetString() const
{
	return BvDebugStringIdTable::GetInstance().GetString(m_Id);
}
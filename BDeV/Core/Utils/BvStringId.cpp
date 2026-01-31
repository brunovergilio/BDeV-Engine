#include "BvStringId.h"
#include "BDeV/Core/Container/BvRobinMap.h"


BvStringId::BvStringId(const char* pId)
{
	if (!pId)
	{
		*this = Empty();
		return;
	}

	u32 len = static_cast<u32>(std::char_traits<char>::length(pId));
	m_Id = MurmurHash64A(pId, len);
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
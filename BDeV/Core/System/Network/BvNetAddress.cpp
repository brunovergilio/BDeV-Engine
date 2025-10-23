#include "BvNetAddress.h"


BvNetAddress::BvNetAddress()
	: m_IPv6({})
{
	BV_NETWORK_INIT;
}


BvNetAddress::BvNetAddress(const BvNetAddress& rhs)
{
	if (rhs.m_Family == NetAddressFamily::kIPv4)
	{
		m_IPv4 = rhs.m_IPv4;
	}
	else if (rhs.m_Family == NetAddressFamily::kIPv6)
	{
		m_IPv6 = rhs.m_IPv6;
	}
}


BvNetAddress::BvNetAddress(BvNetAddress&& rhs) noexcept
{
	if (rhs.m_Family == NetAddressFamily::kIPv4)
	{
		m_IPv4 = rhs.m_IPv4;
	}
	else if (rhs.m_Family == NetAddressFamily::kIPv6)
	{
		m_IPv6 = rhs.m_IPv6;
	}
}


BvNetAddress& BvNetAddress::operator=(const BvNetAddress& rhs)
{
	if (this != &rhs)
	{
		if (rhs.m_Family == NetAddressFamily::kIPv4)
		{
			m_IPv4 = rhs.m_IPv4;
		}
		else if (rhs.m_Family == NetAddressFamily::kIPv6)
		{
			m_IPv6 = rhs.m_IPv6;
		}
	}

	return *this;
}


BvNetAddress& BvNetAddress::operator=(BvNetAddress&& rhs) noexcept
{
	if (this != &rhs)
	{
		if (rhs.m_Family == NetAddressFamily::kIPv4)
		{
			m_IPv4 = rhs.m_IPv4;
		}
		else if (rhs.m_Family == NetAddressFamily::kIPv6)
		{
			m_IPv6 = rhs.m_IPv6;
		}
	}

	return *this;
}


BvNetAddress::BvNetAddress(NetAddressFamily af, u16 port)
	: m_Family(af), m_Port(htons(port))
{
	BV_NETWORK_INIT;
}


BvNetAddress::BvNetAddress(NetAddressFamily af, u16 port, const char* pAddress)
{
	BV_NETWORK_INIT;

	Set(af, port, pAddress);
}


BvNetAddress::~BvNetAddress()
{
}


void BvNetAddress::GetAddressBytes(void* pAddress) const
{
	if (m_Family == NetAddressFamily::kIPv4)
	{
		memcpy(pAddress, &m_IPv4.sin_addr, sizeof(m_IPv4.sin_addr));
	}
	else if (m_Family == NetAddressFamily::kIPv6)
	{
		memcpy(pAddress, &m_IPv6.sin6_addr, sizeof(m_IPv6.sin6_addr));
	}
}


void BvNetAddress::GetAddressString(char* pBuffer, u32 bufferSize) const
{
	if (!inet_ntop(i32(m_Family), m_Family == NetAddressFamily::kIPv4 ? (const void*)&m_IPv4.sin_addr : (const void*)&m_IPv6.sin6_addr, pBuffer, bufferSize))
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}
}


bool BvNetAddress::Set(NetAddressFamily af, u16 port, const char* pAddress)
{
	BV_ASSERT(af != NetAddressFamily::kUnknown, "Invalid address family");
	if (inet_pton(i32(af), pAddress, af == NetAddressFamily::kIPv4 ? (void*)&m_IPv4.sin_addr : (void*)&m_IPv6.sin6_addr))
	{
		m_Family = af;
		m_Port = htons(port);
	}
	else
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	return true;
}


bool BvNetAddress::IsValid() const
{
	return m_Family != NetAddressFamily::kUnknown;
}


bool BvNetAddress::IsAny() const
{
	if (!IsValid())
	{
		return false;
	}
	
	if (m_Family == NetAddressFamily::kIPv4)
	{
		return m_IPv4.sin_addr.S_un.S_addr == in4addr_any.S_un.S_addr;
	}
	else
	{
		return IN6_IS_ADDR_UNSPECIFIED(&m_IPv6.sin6_addr);
	}
}


bool BvNetAddress::IsLoopBack() const
{
	if (!IsValid())
	{
		return false;
	}

	if (m_Family == NetAddressFamily::kIPv4)
	{
		return m_IPv4.sin_addr.S_un.S_addr == in4addr_loopback.S_un.S_addr;
	}
	else
	{
		return IN6_IS_ADDR_LOOPBACK(&m_IPv6.sin6_addr);
	}
}


bool BvNetAddress::IsMulticast() const
{
	if (!IsValid())
	{
		return false;
	}

	if (m_Family == NetAddressFamily::kIPv4)
	{
		auto ipv4 = ntohl(m_IPv4.sin_addr.S_un.S_addr);
		return (ipv4 >= 0xE0000000UL && ipv4 <= 0xEFFFFFFFUL);
	}
	else
	{
		return IN6_IS_ADDR_MULTICAST(&m_IPv6.sin6_addr);
	}
}


bool BvNetAddress::IsExternal() const
{
	return IsValid() && !IsAny() && !IsLoopBack() && !IsMulticast();
}


const BvNetAddress BvNetAddress::AsAny(NetAddressFamily af, u16 port)
{
	return BvNetAddress(af, port);
}


const BvNetAddress BvNetAddress::AsLoopBack(NetAddressFamily af /*= AddressFamily::kUnknown*/, u16 port /*= 0*/)
{
	BV_ASSERT(af != NetAddressFamily::kUnknown, "Invalid address family");
	BvNetAddress addr;
	if (af == NetAddressFamily::kIPv4)
	{
		addr.m_IPv4.sin_addr = in4addr_loopback;
	}
	else
	{
		addr.m_IPv6.sin6_addr = in6addr_loopback;
	}
	addr.m_Family = af;
	addr.m_Port = htons(port);

	return addr;
}


bool BvNetAddress::Resolve(BvNetAddressList& addressList, const char* pHostName, const char* pService /*= nullptr*/, NetAddressFamily af /*= AddressFamily::kUnknown*/,
	SocketType socketType /*= SocketType::kUnknown*/, bool isNumericHost /*= false*/, bool isNumericService /*= false*/)
{
	BV_NETWORK_INIT;

	addrinfo hints{};
	if (af != NetAddressFamily::kUnknown)
	{
		hints.ai_family = i32(af);
	}
	
	if (socketType != SocketType::kUnknown)
	{
		hints.ai_socktype = i32(socketType);
	}

	if (isNumericHost)
	{
		hints.ai_flags |= AI_NUMERICHOST;
	}

	if (isNumericService)
	{
		hints.ai_flags |= AI_NUMERICSERV;
	}

	if (!pHostName)
	{
		hints.ai_flags |= AI_PASSIVE;
	}

	return addressList.Resolve(pHostName, pService, hints);
}


bool operator==(const BvNetAddress& lhs, const BvNetAddress& rhs)
{
	return !memcmp(&lhs, &rhs, sizeof(BvNetAddress));
}



BvNetAddressList::BvNetAddressList()
{
}


BvNetAddressList::~BvNetAddressList()
{
	Reset();
}


bool BvNetAddressList::GetNext(BvNetAddress& address)
{
	while (m_pCurrAddress)
	{
		if ((m_pCurrAddress->ai_family != AF_INET && m_pCurrAddress->ai_family != AF_INET6)
			|| (m_pCurrAddress->ai_protocol != IPPROTO_TCP && m_pCurrAddress->ai_protocol != IPPROTO_UDP)
			|| (m_pCurrAddress->ai_socktype != SOCK_STREAM && m_pCurrAddress->ai_socktype != SOCK_DGRAM))
		{
			m_pCurrAddress = m_pCurrAddress->ai_next;
			continue;
		}

		if (m_pCurrAddress->ai_family == AF_INET)
		{
			*reinterpret_cast<sockaddr_in*>(&address) = *reinterpret_cast<sockaddr_in*>(m_pCurrAddress->ai_addr);
		}
		else if (m_pCurrAddress->ai_family == AF_INET6)
		{
			*reinterpret_cast<sockaddr_in6*>(&address) = *reinterpret_cast<sockaddr_in6*>(m_pCurrAddress->ai_addr);
		}

		return true;
	}

	return false;
}


bool BvNetAddressList::Resolve(const char* pHostName, const char* pServiceName, const addrinfo& hints)
{
	Reset();

	if (getaddrinfo(pHostName, pServiceName, &hints, &m_pAddresses))
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	m_pCurrAddress = m_pAddresses;

	return true;
}


void BvNetAddressList::Reset()
{
	if (m_pAddresses)
	{
		freeaddrinfo(m_pAddresses);
		m_pAddresses = nullptr;
		m_pCurrAddress = nullptr;
	}
}
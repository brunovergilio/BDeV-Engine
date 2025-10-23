#pragma once


#include "BvNetworkCommon.h"


class BvNetAddressList;


class BvNetAddress
{
	friend class BvNetSocket;

public:
	BvNetAddress();
	BvNetAddress(const BvNetAddress& rhs);
	BvNetAddress(BvNetAddress&& rhs) noexcept;
	BvNetAddress& operator=(const BvNetAddress& rhs);
	BvNetAddress& operator=(BvNetAddress&& rhs) noexcept;
	BvNetAddress(NetAddressFamily af, u16 port);
	BvNetAddress(NetAddressFamily af, u16 port, const char* pAddress);
	~BvNetAddress();

	BV_INLINE NetAddressFamily GetAddressFamily() const { return m_Family; }
	BV_INLINE u16 GetPort() const { return ntohs(m_Port); }
	void GetAddressBytes(void* pAddress) const;
	void GetAddressString(char* pBuffer, u32 bufferSize) const;

	bool Set(NetAddressFamily af, u16 port, const char* pAddress);

	bool IsValid() const;
	bool IsAny() const;
	bool IsLoopBack() const;
	bool IsMulticast() const;
	bool IsExternal() const;

	static const BvNetAddress AsAny(NetAddressFamily af = NetAddressFamily::kUnknown, u16 port = 0);
	static const BvNetAddress AsLoopBack(NetAddressFamily af = NetAddressFamily::kUnknown, u16 port = 0);

	static bool Resolve(BvNetAddressList& addressList, const char* pHostName, const char* pService = nullptr, NetAddressFamily af = NetAddressFamily::kUnknown,
		SocketType socketType = SocketType::kUnknown, bool isNumericHost = false, bool isNumericService = false);

	friend bool operator==(const BvNetAddress& lhs, const BvNetAddress& rhs);

private:
	union
	{
		struct
		{
			NetAddressFamily m_Family;
			u16 m_Port;
		};
		sockaddr m_SockAddr;
		sockaddr_in m_IPv4;
		sockaddr_in6 m_IPv6{};
	};
};


class BvNetAddressList
{
	BV_NOCOPYMOVE(BvNetAddressList);

	friend class BvNetAddress;

public:
	BvNetAddressList();
	~BvNetAddressList();

	bool GetNext(BvNetAddress& address);

private:
	bool Resolve(const char* pHostName, const char* pServiceName, const addrinfo& hints);
	void Reset();

private:
	addrinfo* m_pAddresses = nullptr;
	addrinfo* m_pCurrAddress = nullptr;
};
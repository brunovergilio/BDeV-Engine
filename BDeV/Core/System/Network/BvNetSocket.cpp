#include "BvNetSocket.h"


template<typename T, i32 Level = SOL_SOCKET>
i32 SetSocketOption(OSSocketHandle socket, i32 opt, const T& value)
{
	auto pVal = reinterpret_cast<const char*>(&value);
	i32 size = sizeof(T);
	return setsockopt(socket, Level, opt, pVal, size);
}


template<typename T, i32 Level = SOL_SOCKET>
i32 GetSocketOption(OSSocketHandle socket, i32 opt, T& value)
{
	auto pVal = reinterpret_cast<char*>(&value);
	i32 size = sizeof(T);
	return getsockopt(socket, Level, opt, pVal, &size);
}


BvNetSocket::BvNetSocket(OSSocketHandle handle)
	: m_Socket(handle)
{
	if (m_Socket == INVALID_SOCKET)
	{
		BV_SYS_FATAL(BV_SOCKET_ERROR_CODE);
	}
}


BvNetSocket::BvNetSocket()
{
	BV_NETWORK_INIT;
}


BvNetSocket::BvNetSocket(NetAddressFamily af, SocketType type)
{
	BV_NETWORK_INIT;

	Create(af, type);
}


BvNetSocket::BvNetSocket(BvNetSocket&& rhs) noexcept
	: m_Socket(rhs.m_Socket)
{
	rhs.m_Socket = kNullOSSocketHandle;
}


BvNetSocket& BvNetSocket::operator=(BvNetSocket&& rhs) noexcept
{
	if (this != &rhs)
	{
		Close();

		m_Socket = rhs.m_Socket;
		rhs.m_Socket = kNullOSSocketHandle;
	}

	return *this;
}


BvNetSocket::~BvNetSocket()
{
	Close();
}


bool BvNetSocket::Bind(const BvNetAddress& address)
{
	if (bind(m_Socket, &address.m_SockAddr, GetAddressSize(address)) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	return true;
}


bool BvNetSocket::Connect(const BvNetAddress& address)
{
	if (connect(m_Socket, &address.m_SockAddr, GetAddressSize(address)) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	return true;
}


bool BvNetSocket::Listen(i32 backlog)
{
	if (listen(m_Socket, backlog) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	return true;
}


BvNetSocket BvNetSocket::Accept(BvNetAddress* pAddress)
{
	i32 size = 0;
	i32* pSize = nullptr;
	sockaddr* pSockAddr = nullptr;
	if (pAddress)
	{
		pSockAddr = &pAddress->m_SockAddr;
		pSize = &size;
	}
	auto newSocket = accept(m_Socket, pSockAddr, pSize);
	if (newSocket == INVALID_SOCKET)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}

	return BvNetSocket(newSocket);
}


i32 BvNetSocket::Recv(void* pBuffer, i32 maxSize)
{
	auto count = recv(m_Socket, reinterpret_cast<char*>(pBuffer), maxSize, 0);
	if (count == 0)
	{
		Close();
	}
	else if (count < 0)
	{
		auto error = BV_SOCKET_ERROR_CODE;
		if (error != WSAEWOULDBLOCK)
		{
			BV_SYS_ERROR(error);
		}
	}

	return count;
}


i32 BvNetSocket::RecvFrom(BvNetAddress& address, void* pBuffer, i32 maxSize)
{
	i32 size = 0;
	auto count = recvfrom(m_Socket, reinterpret_cast<char*>(pBuffer), maxSize, 0, &address.m_SockAddr, &size);
	if (count == 0)
	{
		Close();
	}
	else if (count < 0)
	{
		auto error = BV_SOCKET_ERROR_CODE;
		if (error != WSAEWOULDBLOCK)
		{
			BV_SYS_ERROR(error);
		}
	}

	return count;
}


i32 BvNetSocket::Send(const void* pBuffer, i32 maxSize)
{
	auto count = send(m_Socket, reinterpret_cast<const char*>(pBuffer), maxSize, 0);
	if (count == 0)
	{
		Close();
	}
	else if (count < 0)
	{
		auto error = BV_SOCKET_ERROR_CODE;
		if (error != WSAEWOULDBLOCK)
		{
			BV_SYS_ERROR(error);
		}
	}

	return count;
}


i32 BvNetSocket::SendTo(const BvNetAddress& address, const void* pBuffer, i32 maxSize)
{
	auto count = sendto(m_Socket, reinterpret_cast<const char*>(pBuffer), maxSize, 0, &address.m_SockAddr, GetAddressSize(address));
	if (count == 0)
	{
		Close();
	}
	else if (count < 0)
	{
		auto error = BV_SOCKET_ERROR_CODE;
		if (error != WSAEWOULDBLOCK)
		{
			BV_SYS_ERROR(error);
		}
	}

	return count;
}


i32 BvNetSocket::WaitForSockets(i32 count, BvNetSocket* pSockets, u32 timeoutMs)
{
	BV_ASSERT(count <= FD_SETSIZE, "count can't be larger than FD_SETSIZE - re-#define FD_SETSIZE");

	i32 events = 0;
	fd_set set;
	FD_ZERO(&set);
	SOCKET maxSocket = 0;
	for (auto i = 0; i < count; ++i)
	{
		FD_SET(pSockets[i].m_Socket, &set);
#if !BV_PLATFORM_IS_WIN32
		maxSocket = std::max(maxSocket, pSockets[i].m_Socket);
#endif
	}

	timeval tm{ 0, timeoutMs == kU32Max ? kI32Max : i32(timeoutMs) };
	events = select(maxSocket + 1, &set, nullptr, nullptr, &tm);
	if (events == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return -1;
	}

	i32 currHandled = 0;
	i32 swapPos = count - 1;
	for (auto i = 0; currHandled < events || i < count; )
	{
		if (FD_ISSET(pSockets[i].m_Socket, &set))
		{
			i++;
			currHandled++;
		}
		else
		{
			std::swap(pSockets[i], pSockets[swapPos--]);
		}
	}

	return events;
}


void BvNetSocket::Create(NetAddressFamily af, SocketType type)
{
	m_Socket = socket(i32(af), i32(type), 0);
	if (m_Socket == INVALID_SOCKET)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}
}

void BvNetSocket::Close()
{
	if (m_Socket != kNullOSSocketHandle)
	{
#if BV_PLATFORM_IS_WIN32
		if (closesocket(m_Socket) == SOCKET_ERROR)
#else
		if (close(m_Socket) == SOCKET_ERROR)
#endif
		{
			BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		}
		m_Socket = kNullOSSocketHandle;
	}
}


bool BvNetSocket::GetRemoteAddress(BvNetAddress& address)
{
	i32 size = 0;
	if (getpeername(m_Socket, &address.m_SockAddr, &size) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}
	
	return true;
}


bool BvNetSocket::GetLocalAddress(BvNetAddress& address)
{
	i32 size = 0;
	if (getsockname(m_Socket, &address.m_SockAddr, &size) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	return true;
}


void BvNetSocket::SetNonBlocking(bool enable)
{
#if BV_PLATFORM_IS_WIN32
	u_long val = enable ? 1 : 0;
	if (ioctlsocket(m_Socket, FIONBIO, &val) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}
#else
#endif
}


void BvNetSocket::SetReuseAddress(bool enable)
{
	if (SetSocketOption(m_Socket, SO_REUSEADDR, i32(enable)) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}
}


bool BvNetSocket::GetReuseAddress() const
{
	i32 val = 0;
	if (GetSocketOption(m_Socket, SO_REUSEADDR, val) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	return bool(val);
}


void BvNetSocket::SetRecvBufferSize(i32 size)
{
	if (SetSocketOption(m_Socket, SO_RCVBUF, size) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}
}


i32 BvNetSocket::GetRecvBufferSize() const
{
	i32 size = 0;
	if (GetSocketOption(m_Socket, SO_RCVBUF, size) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}

	return size;
}

void BvNetSocket::SetSendBufferSize(i32 size)
{
	if (SetSocketOption(m_Socket, SO_SNDBUF, size) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}
}

i32 BvNetSocket::GetSendBufferSize() const
{
	i32 size = 0;
	if (GetSocketOption(m_Socket, SO_SNDBUF, size) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}

	return size;
}


void BvNetSocket::SetRecvTimeout(u32 timeoutInMs)
{
#if BV_PLATFORM_IS_WIN32
	if (SetSocketOption(m_Socket, SO_RCVTIMEO, timeoutInMs) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}
#else
#endif
}

void BvNetSocket::SetSendTimeout(u32 timeoutInMs)
{
#if BV_PLATFORM_IS_WIN32
	if (SetSocketOption(m_Socket, SO_SNDTIMEO, timeoutInMs) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}
#else
#endif
}


void BvNetSocket::SetKeepAlive(bool enable)
{
	if (SetSocketOption(m_Socket, SO_KEEPALIVE, i32(enable)) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}
}


bool BvNetSocket::GetKeepAlive() const
{
	i32 val = 0;
	if (GetSocketOption(m_Socket, SO_KEEPALIVE, val) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	return bool(val);
}


void BvNetSocket::SetTCPNoDelay(bool enable)
{
	if (SetSocketOption<i32, IPPROTO_TCP>(m_Socket, TCP_NODELAY, i32(enable)) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
	}
}


bool BvNetSocket::GetTCPNoDelay() const
{
	i32 val = 0;
	if (GetSocketOption<i32, IPPROTO_TCP>(m_Socket, TCP_NODELAY, val) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	return bool(val);
}


bool BvNetSocket::JoinMulticastGroup(const BvNetAddress& address, const BvNetAddress* pSourceAddress, u32 interfaceId)
{
	i32 result = 0;
	if (pSourceAddress)
	{
		group_source_req gr{};
		gr.gsr_interface = interfaceId;
		memcpy(&gr.gsr_group, &address.m_SockAddr, GetAddressSize(address));
		memcpy(&gr.gsr_source, &pSourceAddress->m_SockAddr, GetAddressSize(*pSourceAddress));
		result = SetSocketOption<group_source_req, IPPROTO_IP>(m_Socket, MCAST_JOIN_SOURCE_GROUP, gr);
	}
	else
	{
		group_req gr{};
		gr.gr_interface = interfaceId;
		memcpy(&gr.gr_group, &address.m_SockAddr, GetAddressSize(address));
		result = SetSocketOption<group_req, IPPROTO_IP>(m_Socket, MCAST_JOIN_GROUP, gr);
	}

	if (result == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	return true;
}


bool BvNetSocket::LeaveMulticastGroup(const BvNetAddress& address, const BvNetAddress* pSourceAddress, u32 interfaceId)
{
	i32 result = 0;
	if (pSourceAddress)
	{
		group_source_req gr{};
		gr.gsr_interface = interfaceId;
		memcpy(&gr.gsr_group, &address.m_SockAddr, GetAddressSize(address));
		memcpy(&gr.gsr_source, &pSourceAddress->m_SockAddr, GetAddressSize(*pSourceAddress));
		result = SetSocketOption<group_source_req, IPPROTO_IP>(m_Socket, MCAST_LEAVE_SOURCE_GROUP, gr);
	}
	else
	{
		group_req gr{};
		gr.gr_interface = interfaceId;
		memcpy(&gr.gr_group, &address.m_SockAddr, GetAddressSize(address));
		result = SetSocketOption<group_req, IPPROTO_IP>(m_Socket, MCAST_LEAVE_GROUP, gr);
	}

	if (result == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	return true;
}

bool BvNetSocket::BlockMulticastSource(const BvNetAddress& address, const BvNetAddress& sourceAddress, u32 interfaceId)
{
	group_source_req gr{};
	gr.gsr_interface = interfaceId;
	memcpy(&gr.gsr_group, &address.m_SockAddr, GetAddressSize(address));
	memcpy(&gr.gsr_source, &sourceAddress.m_SockAddr, GetAddressSize(sourceAddress));

	if (SetSocketOption<group_source_req, IPPROTO_IP>(m_Socket, MCAST_BLOCK_SOURCE, gr) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	return true;
}

bool BvNetSocket::UnblockMulticastSource(const BvNetAddress& address, const BvNetAddress& sourceAddress, u32 interfaceId)
{
	group_source_req gr{};
	gr.gsr_interface = interfaceId;
	memcpy(&gr.gsr_group, &address.m_SockAddr, GetAddressSize(address));
	memcpy(&gr.gsr_source, &sourceAddress.m_SockAddr, GetAddressSize(sourceAddress));

	if (SetSocketOption<group_source_req, IPPROTO_IP>(m_Socket, MCAST_BLOCK_SOURCE, gr) == SOCKET_ERROR)
	{
		BV_SYS_ERROR(BV_SOCKET_ERROR_CODE);
		return false;
	}

	return true;
}


i32 BvNetSocket::GetAddressSize(const BvNetAddress& address)
{
	return address.m_Family == NetAddressFamily::kIPv6 ? sizeof(address.m_IPv6) : sizeof(address.m_IPv4);
}
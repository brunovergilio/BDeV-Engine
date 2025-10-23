#pragma once


#include "BvNetAddress.h"


class BvNetSocket
{
	BV_NOCOPY(BvNetSocket);

	static constexpr i32 kDefaultBacklogSize = 32;

	BvNetSocket(OSSocketHandle handle);

public:
	BvNetSocket();
	BvNetSocket(NetAddressFamily af, SocketType type);
	BvNetSocket(BvNetSocket&& rhs) noexcept;
	BvNetSocket& operator=(BvNetSocket&& rhs) noexcept;
	~BvNetSocket();

	bool Bind(const BvNetAddress& address);
	bool Connect(const BvNetAddress& address);

	bool Listen(i32 backlog = kDefaultBacklogSize);
	BvNetSocket Accept(BvNetAddress* pAddress = nullptr);

	i32 Recv(void* pBuffer, i32 maxSize);
	i32 RecvFrom(BvNetAddress& address, void* pBuffer, i32 maxSize);

	i32 Send(const void* pBuffer, i32 maxSize);
	i32 SendTo(const BvNetAddress& address, const void* pBuffer, i32 maxSize);

	template<typename T> i32 Recv(T& data) { return Recv(&data, sizeof(T)); }
	template<typename T> i32 RecvFrom(const BvNetAddress& address, T& data) { return RecvFrom(address, &data, sizeof(T)); }
	template<typename T> i32 Send(const T& data) { return Send(&data, sizeof(T)); }
	template<typename T> i32 SendTo(const BvNetAddress& address, const T& data) { return SendTo(address, &data, sizeof(T)); }

	static i32 WaitForSockets(i32 count, BvNetSocket* pSockets, u32 timeoutMs = kU32Max);

	void Create(NetAddressFamily af, SocketType type);
	void Close();

	bool GetRemoteAddress(BvNetAddress& address);
	bool GetLocalAddress(BvNetAddress& address);

	// Socket options
	void SetNonBlocking(bool enable);

	void SetReuseAddress(bool enable);
	bool GetReuseAddress() const;

	void SetRecvBufferSize(i32 size);
	i32 GetRecvBufferSize() const;

	void SetSendBufferSize(i32 size);
	i32 GetSendBufferSize() const;

	void SetRecvTimeout(u32 timeoutInMs);
	void SetSendTimeout(u32 timeoutInMs);

	void SetKeepAlive(bool enable);
	bool GetKeepAlive() const;

	void SetTCPNoDelay(bool enable);
	bool GetTCPNoDelay() const;

	bool JoinMulticastGroup(const BvNetAddress& address, const BvNetAddress* pSourceAddress = nullptr, u32 interfaceId = 0);
	bool LeaveMulticastGroup(const BvNetAddress& address, const BvNetAddress* pSourceAddress = nullptr, u32 interfaceId = 0);
	bool BlockMulticastSource(const BvNetAddress& address, const BvNetAddress& sourceAddress, u32 interfaceId = 0);
	bool UnblockMulticastSource(const BvNetAddress& address, const BvNetAddress& sourceAddress, u32 interfaceId = 0);

	BV_INLINE OSSocketHandle GetHandle() const { return m_Socket; }
	BV_INLINE operator bool() const { return m_Socket != kNullOSSocketHandle; }

private:
	i32 GetAddressSize(const BvNetAddress& address);

private:
	OSSocketHandle m_Socket = kNullOSSocketHandle;
};
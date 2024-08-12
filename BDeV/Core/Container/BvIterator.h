#pragma once


#include <utility>


template<class Type>
class RandomIterator
{
	RandomIterator() {}

public:
	RandomIterator(Type * const pData) : m_pData(pData) {}
	RandomIterator(const RandomIterator & rhs) : m_pData(rhs.m_pData) {}
	RandomIterator & operator =(const RandomIterator & rhs) { if (this != &rhs) { m_pData = rhs.m_pData; } return *this; }

	bool operator ==(const RandomIterator & rhs) const { return m_pData == rhs.m_pData; }
	bool operator !=(const RandomIterator & rhs) const { return m_pData != rhs.m_pData; }
	bool operator < (const RandomIterator & rhs) const { return m_pData < rhs.m_pData; }
	bool operator <=(const RandomIterator & rhs) const { return m_pData <= rhs.m_pData; }
	bool operator > (const RandomIterator & rhs) const { return m_pData > rhs.m_pData; }
	bool operator >=(const RandomIterator & rhs) const { return m_pData >= rhs.m_pData; }

	Type & operator *() { return *m_pData; }
	Type * operator ->() { return m_pData; }
	Type & operator [](const size_t index) { return m_pData[index]; }
	const Type & operator [](const size_t index) const { return m_pData[index]; }
	void Swap(RandomIterator & rhs) { Type pTmp = *m_pData; *m_pData = *rhs.m_pData; *rhs.m_pData = *pTmp; }

	RandomIterator operator++(int) { RandomIterator it(*this); m_pData++; return it; }
	RandomIterator & operator++() { m_pData++; return *this; }

	RandomIterator operator--(int) { RandomIterator it(*this); m_pData--; return it; }
	RandomIterator & operator--() { m_pData--; return *this; }

	RandomIterator & operator+=(const size_t n) { m_pData += n; return *this; }
	RandomIterator & operator-=(const size_t n) { m_pData -= n; return *this; }

	friend RandomIterator operator+(const RandomIterator & lhs, const size_t n) { RandomIterator it; it.m_pData = lhs.m_pData + n; return it; }
	friend RandomIterator operator-(const RandomIterator & lhs, const size_t n) { RandomIterator it; it.m_pData = lhs.m_pData - n; return it; }

	friend RandomIterator operator+(const size_t n, const RandomIterator & rhs) { RandomIterator it; it.m_pData = rhs.m_pData + n; return it; }
	friend RandomIterator operator-(const size_t n, const RandomIterator & rhs) { RandomIterator it; it.m_pData = rhs.m_pData - n; return it; }

	friend size_t operator-(const RandomIterator & lhs, const RandomIterator & rhs) { return lhs.m_pData - rhs.m_pData; }

	operator RandomIterator<const Type>() { return RandomIterator<const Type>(m_pData); }

private:
	Type * m_pData = nullptr;
};


template<class Type>
class RandomReverseIterator
{
	RandomReverseIterator() {}

public:
	RandomReverseIterator(Type * const pData) : m_pData(pData) {}
	RandomReverseIterator(const RandomReverseIterator & rhs) : m_pData(rhs.m_pData) {}
	RandomReverseIterator & operator =(const RandomReverseIterator & rhs) { if (this != &rhs) { m_pData = rhs.m_pData; } return *this; }

	bool operator ==(const RandomReverseIterator & rhs) const { return m_pData == rhs.m_pData; }
	bool operator !=(const RandomReverseIterator & rhs) const { return m_pData != rhs.m_pData; }
	bool operator < (const RandomReverseIterator & rhs) const { return m_pData < rhs.m_pData; }
	bool operator <=(const RandomReverseIterator & rhs) const { return m_pData <= rhs.m_pData; }
	bool operator > (const RandomReverseIterator & rhs) const { return m_pData > rhs.m_pData; }
	bool operator >=(const RandomReverseIterator & rhs) const { return m_pData >= rhs.m_pData; }

	Type & operator *() { return *m_pData; }
	Type * operator ->() { return m_pData; }
	Type & operator [](const size_t index) { m_pData[index]; }
	const Type & operator [](const size_t index) const { m_pData[index]; }
	void Swap(RandomReverseIterator & rhs) { Type pTmp = *m_pData; *m_pData = *rhs.m_pData; *rhs.m_pData = *pTmp; }

	RandomReverseIterator operator++(int) { RandomReverseIterator it(*this); m_pData--; return it; }
	RandomReverseIterator & operator++() { m_pData--; return *this; }

	RandomReverseIterator operator--(int) { RandomReverseIterator it(*this); m_pData++; return it; }
	RandomReverseIterator & operator--() { m_pData++; return *this; }

	RandomReverseIterator & operator+=(const size_t n) { m_pData -= n; return *this; }
	RandomReverseIterator & operator-=(const size_t n) { m_pData += n; return *this; }

	friend RandomReverseIterator operator+(const RandomReverseIterator & lhs, const size_t n) { RandomReverseIterator it; it.m_pData = lhs.m_pData - n; return it; }
	friend RandomReverseIterator operator-(const RandomReverseIterator & lhs, const size_t n) { RandomReverseIterator it; it.m_pData = lhs.m_pData + n; return it; }

	friend RandomReverseIterator operator+(const size_t n, const RandomReverseIterator & rhs) { RandomReverseIterator it; it.m_pData = rhs.m_pData - n; return it; }
	friend RandomReverseIterator operator-(const size_t n, const RandomReverseIterator & rhs) { RandomReverseIterator it; it.m_pData = rhs.m_pData + n; return it; }

	friend size_t operator-(const RandomReverseIterator & lhs, const RandomReverseIterator & rhs) { return rhs.m_pData - lhs.m_pData; }

	operator RandomReverseIterator<const Type>() { return RandomReverseIterator<const Type>(m_pData); }

private:
	Type * m_pData = nullptr;
};


template<typename Type, bool Const = false>
class RobinIterator
{
	using reference = typename std::conditional_t<Const, Type const &, Type &>;
	using pointer = typename std::conditional_t<Const, Type const *, Type *>;
	
	friend class RobinIterator<Type, !Const>;

	RobinIterator() {}

public:
	RobinIterator(const Type * const pData, Type * const pCurr, size_t * const pHashes, const size_t * const pSize)
		: m_pData(pData), m_pCurr(pCurr), m_pHashes(pHashes), m_pSize(pSize)
	{
		while (m_pCurr - m_pData < *m_pSize && *m_pHashes == 0)
		{
			m_pCurr++;
			m_pHashes++;
		}
	}

	RobinIterator(const RobinIterator & rhs)
		: m_pData(rhs.m_pData), m_pCurr(rhs.m_pCurr), m_pHashes(rhs.m_pHashes), m_pSize(rhs.m_pSize) {}

	RobinIterator & operator =(const RobinIterator & rhs)
	{
		if (this != &rhs)
		{
			m_pData = rhs.m_pData;
			m_pCurr = rhs.m_pCurr;
			m_pHashes = rhs.m_pHashes;
			m_pSize = rhs.m_pSize;
		}
		
		return *this;
	}

	template<bool Enable = Const>
	bool operator ==(const RobinIterator<Type, Enable> & rhs) const { return m_pCurr == rhs.m_pCurr; }
	template<bool Enable = Const>
	bool operator !=(const RobinIterator<Type, Enable> & rhs) const { return m_pCurr != rhs.m_pCurr; }

	template< bool Enable = Const >
	std::enable_if_t<Enable, reference> operator *() const { return *m_pCurr; }

	template< bool Enable = Const >
	std::enable_if_t<!Enable, reference> operator *() { return *m_pCurr; }

	template< bool Enable = Const >
	std::enable_if_t<Enable, pointer> operator ->() const { return m_pCurr; }

	template< bool Enable = Const >
	std::enable_if_t<!Enable, pointer> operator ->() { return m_pCurr; }

	void Swap(RobinIterator & rhs)
	{
		Type * pTmp = *m_pData; *m_pData = *rhs.m_pData; *rhs.m_pData = *pTmp;
		pTmp = *m_pCurr; *m_pCurr = *rhs.m_pCurr; *rhs.m_pCurr = *pTmp;
		size_t * pHashTmp = *m_pHashes; *m_pHashes = *rhs.m_pHashes; *rhs.m_pHashes = *pHashTmp;
		size_t * pSizeTmp = *m_pSize; *m_pSize = *rhs.m_pSize; *rhs.m_pSize = *pSizeTmp;
	}

	RobinIterator operator++(int)
	{
		RobinIterator it(*this);
		while (m_pCurr - m_pData < *m_pSize)
		{
			m_pCurr++;
			m_pHashes++;

			if (*m_pHashes != 0)
			{
				break;
			}
		}

		return it;
	}

	RobinIterator & operator++()
	{
		while (m_pCurr - m_pData < *m_pSize)
		{
			m_pCurr++;
			m_pHashes++;

			if (*m_pHashes != 0)
			{
				break;
			}
		}

		return *this;
	}

	const size_t GetIndex() const { return m_pCurr - m_pData; }

	operator RobinIterator<Type, true>() { return RobinIterator<Type, true>(m_pData, m_pCurr, m_pHashes, m_pSize); }

private:
	const Type * m_pData = nullptr;
	Type * m_pCurr = nullptr;
	size_t * m_pHashes = nullptr;
	const size_t * m_pSize = nullptr;
};
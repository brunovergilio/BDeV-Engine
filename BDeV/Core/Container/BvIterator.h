#pragma once


#include <utility>
#include <iterator>


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
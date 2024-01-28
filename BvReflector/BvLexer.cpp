#include "BvLexer.h"


BvLexer::BvLexer(const char* pData)
	: m_pCurr(pData)
{
	// Make sure we're not being fed an empty string
	if (GetTokenType(*m_pCurr) == Token::Type::kEnd)
	{
		return;
	}

	// Make sure we skip the initial white spaces, if any
	while (IsSpace(*m_pCurr))
	{
		++m_pCurr;
	}
}


BvLexer::BvLexer(const BvLexer& rhs)
	: m_pCurr(rhs.m_pCurr)
{
}


BvLexer& BvLexer::operator=(const BvLexer& rhs)
{
	if (this != &rhs)
	{
		m_pCurr = rhs.m_pCurr;
	}

	return *this;
}


BvLexer::BvLexer(BvLexer&& rhs) noexcept
{
	*this = std::move(rhs);
}


BvLexer& BvLexer::operator=(BvLexer&& rhs) noexcept
{
	if (this != &rhs)
	{
		std::swap(m_pCurr, rhs.m_pCurr);
	}

	return *this;
}


BvLexer::~BvLexer()
{
}


Token BvLexer::GetAndMoveNext()
{
	Token token = Get();
	if (token.m_Type != Token::Type::kEnd)
	{
		m_pCurr += token.m_Length;

		while (IsSpace(*m_pCurr))
		{
			++m_pCurr;
		}
	}

	return token;
}


Token BvLexer::Get() const
{
	if (GetTokenType(*m_pCurr) == Token::Type::kEnd)
	{
		return Token{ nullptr, 0, Token::Type::kEnd };
	}

	auto type = GetTokenType(*m_pCurr);
	if (type == Token::Type::kIdentifier)
	{
		const char* pStart = m_pCurr;
		const char* pEnd = pStart;
		do
		{
			++pEnd;
		} while (IsIdentifier(*pEnd));

		return Token{ pStart, static_cast<u32>(pEnd - pStart), Token::Type::kIdentifier };
	}
	else if (type == Token::Type::kNumber)
	{
		const char* pStart = m_pCurr;
		const char* pEnd = pStart;
		do
		{
			++pEnd;
		} while (IsDigit(*pEnd));

		return Token{ pStart, static_cast<u32>(pEnd - pStart), Token::Type::kNumber };
	}

	return Token{ m_pCurr, 1, type };
}


Token::Type BvLexer::GetTokenType(char c) const
{
	auto type = Token::Type::kUnknown;
	if (IsDigit(c))
	{
		type = Token::Type::kNumber;
	}
	else if (IsAlpha(c))
	{
		type = Token::Type::kIdentifier;
	}
	else
	{
		type = static_cast<Token::Type>(c);
	}

	return type;
}


bool BvLexer::IsSpace(char c) const
{
	return std::isspace(c);
}


bool BvLexer::IsNewLine(char c) const
{
	return c == '\n' || c == '\r';
}


bool BvLexer::IsSpaceOrNewLine(char c) const
{
	return IsSpace(c) || IsNewLine(c);
}


bool BvLexer::IsDigit(char c) const
{
	return std::isdigit(c);
}


bool BvLexer::IsAlpha(char c) const
{
	return std::isalpha(c);
}


bool BvLexer::IsAlphaNum(char c) const
{
	return std::isalnum(c);
}


bool BvLexer::IsIdentifier(char c) const
{
	return IsAlphaNum(c) || c == '_';
}
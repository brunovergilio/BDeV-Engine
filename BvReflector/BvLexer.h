#pragma once


#include "BDeV/BvCore.h"
#include "BDeV/Utils/BvUtils.h"
#include <string>


using TokenView = std::string_view;


struct Token
{
public:
	enum class Type : i8
	{
		kEnd = '\0',
		kNumber = '0',
		kIdentifier = 'a',
		kLeftParenthesis = '(',
		kRightParenthesis = ')',
		kLeftSquareBracket = '[',
		kRightSquareBracket = ']',
		kLeftCurlyBrace = '{',
		kRightCurlyBrace = '}',
		kLessThan = '<',
		kGreaterThan = '>',
		kEqual = '=',
		kPlus = '+',
		kMinus = '-',
		kAsterisk = '*',
		kForwardSlash = '/',
		kBackwardSlash = '\\',
		kHash = '#',
		kDot = '.',
		kComma = ',',
		kColon = ':',
		kSemicolon = ';',
		kTilde = '~',
		kCircumflex = '^',
		kSingleQuote = '\'',
		kDoubleQuote = '\"',
		kPipe = '|',
		kUnknown = -128,
	};

	const char* m_pToken;
	u32 m_Length = 0;
	Type m_Type;

	bool MatchesKeyword(const char* pStr) const
	{
		return m_Type == Type::kIdentifier ? std::char_traits<char>::compare(m_pToken, pStr, m_Length) == 0 : false;
	}
};


class BvLexer
{
public:
	BvLexer(const char* pData);
	BvLexer(const BvLexer& rhs);
	BvLexer(BvLexer&& rhs);
	BvLexer& operator=(const BvLexer& rhs) noexcept;
	BvLexer& operator=(BvLexer&& rhs) noexcept;
	~BvLexer();

	Token GetAndMoveNext();
	Token Get() const;

	Token::Type GetTokenType(char c) const;

	bool IsSpace(char c) const;
	bool IsNewLine(char c) const;
	bool IsSpaceOrNewLine(char c) const;
	bool IsDigit(char c) const;
	bool IsAlpha(char c) const;
	bool IsAlphaNum(char c) const;
	bool IsIdentifier(char c) const;

private:
	const char* m_pCurr = nullptr;
};
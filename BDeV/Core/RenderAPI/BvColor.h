#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/Math/BvMath.h"


class BvColor;


class BvColorI
{
public:
	static const BvColorI Zero;
	static const BvColorI One;
	static const BvColorI White;
	static const BvColorI Gray;
	static const BvColorI Black;
	static const BvColorI Red;
	static const BvColorI Green;
	static const BvColorI Blue;
	static const BvColorI Yellow;
	static const BvColorI Magenta;
	static const BvColorI Cyan;

	static const BvColorI BrightWhite;
	static const BvColorI BrightRed;
	static const BvColorI BrightGreen;
	static const BvColorI BrightBlue;
	static const BvColorI BrightYellow;
	static const BvColorI BrightMagenta;
	static const BvColorI BrightCyan;

	constexpr BvColorI(u8 red = 255, u8 green = 255, u8 blue = 255, u8 alpha = 255)
		: m_Red(red), m_Green(green), m_Blue(blue), m_Alpha(alpha) {}

	constexpr BvColorI(u32 rgba)
		: m_RGBA(rgba) {}
	constexpr BvColorI(i32 rgba)
		: m_RGBA(rgba) {}

	BvColorI(const BvColorI& rhs)
		: m_RGBA(rhs.m_RGBA) {}

	BvColorI& operator=(const BvColorI& rhs)
	{
		if (this != &rhs)
		{
			m_RGBA = rhs.m_RGBA;
		}

		return *this;
	}

	BvColorI(BvColorI&& rhs) noexcept
	{
		*this = std::move(rhs);
	}

	BvColorI& operator=(BvColorI&& rhs) noexcept
	{
		if (this != &rhs)
		{
			m_RGBA = rhs.m_RGBA;
		}

		return *this;
	}

	BvColor ToColor() const;

public:
	union
	{
		struct
		{
			u8 m_Alpha;
			u8 m_Blue;
			u8 m_Green;
			u8 m_Red;
		};
		u32 m_RGBA;
	};
};


class BvColor
{
public:
	static const BvColor Zero;
	static const BvColor One;
	static const BvColor White;
	static const BvColor Gray;
	static const BvColor Black;
	static const BvColor Red;
	static const BvColor Green;
	static const BvColor Blue;
	static const BvColor Yellow;
	static const BvColor Magenta;
	static const BvColor Cyan;

	static const BvColor BrightWhite;
	static const BvColor BrightRed;
	static const BvColor BrightGreen;
	static const BvColor BrightBlue;
	static const BvColor BrightYellow;
	static const BvColor BrightMagenta;
	static const BvColor BrightCyan;

	constexpr BvColor(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f);

	BvColor(const BvColor& rhs);
	BvColor& operator=(const BvColor& rhs);
	BvColor(BvColor&& rhs) noexcept;
	BvColor& operator=(BvColor&& rhs) noexcept;

	BvColorI ToColorI() const;
	void Clamp(f32 min, f32 max);
	BvColor GetClampedValue(f32 min, f32 max) const;
	void Saturate();
	BvColor GetSaturatedValue() const;

public:
	union
	{
		struct
		{
			f32 m_Red;
			f32 m_Green;
			f32 m_Blue;
			f32 m_Alpha;
		};
	};
};
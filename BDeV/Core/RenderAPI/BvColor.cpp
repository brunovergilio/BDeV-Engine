#include "BvColor.h"
#include <cmath>


const BvColorI BvColorI::Zero			= BvColorI(0, 0, 0, 0);
const BvColorI BvColorI::One			= BvColorI();

const BvColorI BvColorI::Black			= BvColorI(0, 0, 0);
const BvColorI BvColorI::White			= BvColorI(192, 192, 192);
const BvColorI BvColorI::Red			= BvColorI(128, 0, 0);
const BvColorI BvColorI::Green			= BvColorI(0, 128, 0);
const BvColorI BvColorI::Blue			= BvColorI(0, 0, 128);
const BvColorI BvColorI::Yellow			= BvColorI(128, 128, 0);
const BvColorI BvColorI::Magenta		= BvColorI(128, 0, 128);
const BvColorI BvColorI::Cyan			= BvColorI(0, 128, 128);

const BvColorI BvColorI::Gray			= BvColorI(128, 128, 128);
const BvColorI BvColorI::BrightWhite	= BvColorI();
const BvColorI BvColorI::BrightRed		= BvColorI(255, 0, 0);
const BvColorI BvColorI::BrightGreen	= BvColorI(0, 255, 0);
const BvColorI BvColorI::BrightBlue		= BvColorI(0, 0, 255);
const BvColorI BvColorI::BrightYellow	= BvColorI(255, 255, 0);
const BvColorI BvColorI::BrightMagenta	= BvColorI(255, 0, 255);
const BvColorI BvColorI::BrightCyan		= BvColorI(0, 255, 255);


BvColor BvColorI::ToColor() const
{
	return BvColor(m_Red / 255.0f, m_Green / 255.0f, m_Blue / 255.0f, m_Alpha / 255.0f);
}


const BvColor BvColor::Zero				= BvColor(0.0f, 0.0f, 0.0f, 0.0f);
const BvColor BvColor::One				= BvColor();

const BvColor BvColor::Black			= BvColor(0.0f, 0.0f, 0.0f);
const BvColor BvColor::White			= BvColor(0.75f, 0.75f, 0.75f);
const BvColor BvColor::Red				= BvColor(0.5f, 0.0f, 0.0f);
const BvColor BvColor::Green			= BvColor(0.0f, 0.5f, 0.0f);
const BvColor BvColor::Blue				= BvColor(0.0f, 0.0f, 0.5f);
const BvColor BvColor::Yellow			= BvColor(0.5f, 0.5f, 0.0f);
const BvColor BvColor::Magenta			= BvColor(0.5f, 0.0f, 0.5f);
const BvColor BvColor::Cyan				= BvColor(0.0f, 0.5f, 0.5f);

const BvColor BvColor::Gray				= BvColor(0.5f, 0.5f, 0.5f);
const BvColor BvColor::BrightWhite		= BvColor();
const BvColor BvColor::BrightRed		= BvColor(1.0f, 0.0f, 0.0f);
const BvColor BvColor::BrightGreen		= BvColor(0.0f, 1.0f, 0.0f);
const BvColor BvColor::BrightBlue		= BvColor(0.0f, 0.0f, 1.0f);
const BvColor BvColor::BrightYellow		= BvColor(1.0f, 1.0f, 0.0f);
const BvColor BvColor::BrightMagenta	= BvColor(1.0f, 0.0f, 1.0f);
const BvColor BvColor::BrightCyan		= BvColor(0.0f, 1.0f, 1.0f);


constexpr BvColor::BvColor(float red, float green, float blue, float alpha)
	: m_Red(red), m_Green(green), m_Blue(blue), m_Alpha(alpha) {}

BvColor::BvColor(const BvColor& rhs)
	: m_Red(rhs.m_Red), m_Green(rhs.m_Green), m_Blue(rhs.m_Blue), m_Alpha(rhs.m_Alpha) {}

BvColor& BvColor::operator=(const BvColor& rhs)
{
	if (this != &rhs)
	{
		m_Red = rhs.m_Red;
		m_Green = rhs.m_Green;
		m_Blue = rhs.m_Blue;
		m_Alpha = rhs.m_Alpha;
	}

	return *this;
}

BvColor::BvColor(BvColor&& rhs) noexcept
{
	*this = std::move(rhs);
}

BvColor& BvColor::operator=(BvColor&& rhs) noexcept
{
	if (this != &rhs)
	{
		m_Red = rhs.m_Red;
		m_Green = rhs.m_Green;
		m_Blue = rhs.m_Blue;
		m_Alpha = rhs.m_Alpha;
	}

	return *this;
}

BvColor::BvColor(const BvVec& rhs)
	: m_Vec(rhs) {}


BvColorI BvColor::ToColorI() const
{
	auto color = GetSaturatedValue();
	return BvColorI(u8(m_Red * 255.0f), u8(m_Green * 255.0f), u8(m_Blue * 255.0f), u8(m_Alpha * 255.0f));
}

void BvColor::Clamp(f32 min, f32 max)
{
	*this = GetClampedValue(min, max);
}

BvColor BvColor::GetClampedValue(f32 min, f32 max) const
{
	BvVec vmax(max);
	BvVec vmin(min);

	return vmax.Min(vmin.Max(m_Vec));
}

void BvColor::Saturate()
{
	*this = GetClampedValue(0.0f, 1.0f);
}

BvColor BvColor::GetSaturatedValue() const
{
	return GetClampedValue(0.0f, 1.0f);
}
#pragma once


#include "BvMathDefs.h"


namespace Internal
{
	BV_INLINE float MapAngleMPiToPi(float x)
	{
		float y = x * k1Div2Pi;
		if (y >= 0.0f)
		{
			y = static_cast<float>(static_cast<int>(y + 0.5f));
		}
		else
		{
			y = static_cast<float>(static_cast<int>(y - 0.5f));
		}
		return x - k2Pi * y;
	}
}


BV_INLINE float Sin(float x)
{
	constexpr float k1Div3Fac = 1.0f / 6.0f;
	constexpr float k1Div5Fac = 1.0f / 120.0f;
	constexpr float k1Div7Fac = 1.0f / 5040.0f;

	x = Internal::MapAngleMPiToPi(x);

	if (x > kPiDiv2)
	{
		x = kPi - x;
	}
	else if (x < -kPiDiv2)
	{
		x = -kPi - x;
	}

	// Taylor Series [x - x3/3! + x5/5! - x7/7!]
	float x2 = x * x;
	return x * (1.0f - x2 * (k1Div3Fac + x2 * (k1Div5Fac/* - x2 * (k1Div7Fac)*/)));
}


BV_INLINE float Cos(float x)
{
	constexpr float k1Div2Fac = 1.0f / 2.0f;
	constexpr float k1Div4Fac = 1.0f / 24.0f;
	constexpr float k1Div6Fac = 1.0f / 720.0f;

	x = Internal::MapAngleMPiToPi(x);

	float sign = 1.0f;
	if (x > kPiDiv2)
	{
		x = kPi - x;
		sign = -1.0f;
	}
	else if (x < -kPiDiv2)
	{
		x = -kPi - x;
		sign = -1.0f;
	}

	// Taylor Series [1 - x2/2! + x4/4! - x6/6!]
	float x2 = x * x;
	return 1.0f - x2 * (k1Div2Fac + x2 * (k1Div4Fac/* - x2 * (k1Div6Fac)*/));
}


BV_INLINE float Tan(float x)
{
	constexpr float k1Div3 = 1.0f / 3.0f;
	constexpr float k2Div15 = 2.0f / 15.0f;
	constexpr float k17Div315 = 17.0f / 315.0f;

	x = Internal::MapAngleMPiToPi(x);

	float sign = 1.0f;
	if (x > kPiDiv2)
	{
		x = kPi - x;
		sign = -1.0f;
	}
	else if (x < -kPiDiv2)
	{
		x = -kPi - x;
		sign = -1.0f;
	}

	// Taylor Series [x + x3/3 + 2x5/15 + 17x7/315]
	float x2 = x * x;
	return x * (1.0f + x2 * (k1Div3 + x2 * (k2Div15 + x2 * (k17Div315))));
}


BV_INLINE float ASin(float x)
{
	constexpr float k1Div6 = 1.0f / 6.0f;
	constexpr float k3Div40 = 3.0f / 40.0f;
	constexpr float k5Div112 = 5.0f / 112.0f;

	if (x < -1.0f)
	{
		x = -1.0f;
	}
	if (x > 1.0f)
	{
		x = 1.0f;
	}

	float x2 = x * x;
	return x * (1.0f + x2 * (k1Div6 + x2 * (k3Div40 + x2 * k5Div112)));
	//return x * (1.0f + x2 * k1Div6) + (x * (1.0f - x2)) * 0.5f;
}


BV_INLINE float ACos(float x)
{
	return kPiDiv2 - ASin(x);
}


BV_INLINE float ATan(float x)
{
	// Coefficients for approximation
	constexpr float kCoef1 = 0.2447f;
	constexpr float kCoef2 = 0.6575f;
	constexpr float kCoef3 = 0.9718f;

	if (x < -1.0f)
	{
		return -kPiDiv4 + kCoef1 * (x + 1.0f);
	}
	if (x > 1.0f)
	{
		return kPiDiv4 + kCoef1 * (x - 1.0f);
	}

	// Rational function approximation
	float inv = 1.0f / (1.0f + kCoef3 * x * x);
	return kCoef2 * x * inv;
}
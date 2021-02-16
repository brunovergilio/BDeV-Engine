#pragma once


#include "BvMathDefs.h"


struct Float2
{
	union
	{
		struct { float x, y; };
		float v[2];
	};

	Float2() : x(0.0f), y(0.0f) {}
	explicit Float2(const float x, const float y)
		: x(x), y(y) {}

	void Set(const float x, const float y)
	{
		this->x = x;
		this->y = y;
	}
};


struct Float3
{
	union
	{
		struct { float x, y, z; };
		float v[3];
		Float2 v2;
	};

	Float3() : x(0.0f), y(0.0f), z(0.0f) {}
	explicit Float3(const float x, const float y, const float z)
		: x(x), y(y), z(z) {}

	void Set(const float x, const float y, const float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
};


struct Float4
{
	union
	{
		struct { float x, y, z, w; };
		float v[4];
		Float2 v2;
		Float3 v3;
	};

	Float4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	explicit Float4(const float x, const float y, const float z, const float w = 0.0f)
		: x(x), y(y), z(z), w(w) {}

	void Set(const float x, const float y, const float z, const float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
};


struct Float22
{
	union
	{
		Float2 r[2];
		float m[4];
		struct
		{
			float m0, m1,
				m2, m3;
		};
	};

	Float22() {}
	explicit Float22(const Float2 & r0, const Float2 & r1)
	{
		r[0] = r0;
		r[1] = r1;
	}
};


struct Float33
{
	union
	{
		Float3 r[3];
		float m[9];
		struct
		{
			float m0, m1, m2,
				m3, m4, m5,
				m6, m7, m8;
		};
	};

	Float33() {}
	explicit Float33(const Float3 & r0, const Float3 & r1, const Float3 & r2)
	{
		r[0] = r0;
		r[1] = r1;
		r[2] = r2;
	}
};


struct Float43
{
	union
	{
		Float3 r[4];
		float m[12];
		struct
		{
			float m0, m1, m2,
				m3, m4, m5,
				m6, m7, m8,
				m9, m10, m11;
		};
	};

	Float43() {}
	explicit Float43(const Float3 & r0, const Float3 & r1, const Float3 & r2, const Float3 & r3)
	{
		r[0] = r0;
		r[1] = r1;
		r[2] = r2;
		r[3] = r3;
	}
};


struct Float44
{
	union
	{
		Float4 r[4];
		float m[16];
		struct
		{
			float m0, m1, m2, m3,
				m4, m5, m6, m7,
				m8, m9, m10, m11,
				m12, m13, m14, m15;
		};
	};

	Float44() {}
	explicit Float44(const Float4 & r0, const Float4 & r1, const Float4 & r2, const Float4 & r3)
	{
		r[0] = r0;
		r[1] = r1;
		r[2] = r2;
		r[3] = r3;
	}
};


#if (BV_MATH_USE_TYPE == BV_MATH_TYPE_SIMD)
typedef __m128 BvFastVec;

struct BvFastMat
{
	BvFastVec r[4];
};

typedef __m128 BvFastQuat;
#elif (BV_MATH_USE_TYPE == BV_MATH_TYPE_FPU)
struct BvFastVec
{
	union
	{
		struct
		{
			float x, y, z, w;
		};
		float v[4];
	};

	BvFastVec() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	explicit BvFastVec(const float x, const float y, const float z, const float w = 0.0f)
		: x(x), y(y), z(z), w(w) {}

	void Set(const float x, const float y, const float z, const float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
};

struct BvFastMat
{
	BvFastMat() {}
	explicit BvFastMat(const BvFastVec & r0, const BvFastVec & r1, const BvFastVec & r2, const BvFastVec & r3)
	{
		r[0] = r0;
		r[1] = r1;
		r[2] = r2;
		r[3] = r3;
	}

	BvFastVec r[4];
};

typedef BvFastVec BvFastQuat;

#endif
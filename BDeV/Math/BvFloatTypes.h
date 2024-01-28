#pragma once


#include "BvMathDefs.h"


struct Float2
{
	union
	{
		struct { f32 x, y; };
		f32 v[2];
	};

	constexpr Float2() : x(0.0f), y(0.0f) {}
	constexpr Float2(f32 x, f32 y)
		: x(x), y(y) {}

	void Set(f32 x, f32 y)
	{
		this->x = x;
		this->y = y;
	}
};


struct Float3
{
	union
	{
		struct { f32 x, y, z; };
		f32 v[3];
		Float2 v2;
	};

	constexpr Float3() : x(0.0f), y(0.0f), z(0.0f) {}
	constexpr Float3(f32 x, f32 y, f32 z)
		: x(x), y(y), z(z) {}

	void Set(f32 x, f32 y, f32 z)
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
		struct { f32 x, y, z, w; };
		f32 v[4];
		Float2 v2;
		Float3 v3;
	};

	constexpr Float4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	constexpr Float4(f32 x, f32 y, f32 z, f32 w = 0.0f)
		: x(x), y(y), z(z), w(w) {}

	void Set(f32 x, f32 y, f32 z, f32 w)
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
		Float2 r[2]{};
		f32 m[4];
		struct
		{
			f32 m0, m1,
				m2, m3;
		};
	};

	constexpr Float22() {}
	constexpr Float22(const Float2 & r0, const Float2 & r1)
	{
		r[0] = r0;
		r[1] = r1;
	}
};


struct Float33
{
	union
	{
		Float3 r[3]{};
		f32 m[9];
		struct
		{
			f32 m0, m1, m2,
				m3, m4, m5,
				m6, m7, m8;
		};
	};

	constexpr Float33() {}
	constexpr Float33(const Float3 & r0, const Float3 & r1, const Float3 & r2)
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
		Float3 r[4]{};
		f32 m[12];
		struct
		{
			f32 m0, m1, m2,
				m3, m4, m5,
				m6, m7, m8,
				m9, m10, m11;
		};
	};

	constexpr Float43() {}
	constexpr Float43(const Float3 & r0, const Float3 & r1, const Float3 & r2, const Float3 & r3)
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
		Float4 r[4]{};
		f32 m[16];
		struct
		{
			f32 m0, m1, m2, m3,
				m4, m5, m6, m7,
				m8, m9, m10, m11,
				m12, m13, m14, m15;
		};
	};

	constexpr Float44() {}
	constexpr Float44(const Float4 & r0, const Float4 & r1, const Float4 & r2, const Float4 & r3)
	{
		r[0] = r0;
		r[1] = r1;
		r[2] = r2;
		r[3] = r3;
	}
};


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_SIMD)

using vf32 = __m128;
using crvf32 = vf32;

struct mf32
{
	vf32 r[4];
};

using qf32 = vf32;
using crqf32 = qf32;

#define CRBvVec BvVec
#define CRBvQuat BvQuat

#elif (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_FPU)

struct vf32
{
	union
	{
		struct
		{
			f32 x, y, z, w;
		};
		f32 v[4];
	};

	vf32() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	vf32(f32 x, f32 y, f32 z, f32 w = 0.0f)
		: x(x), y(y), z(z), w(w) {}

	void Set(f32 x, f32 y, f32 z, f32 w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
};
using crvf32 = const vf32&;

struct mf32
{
	mf32() {}
	mf32(const vf32 & r0, const vf32 & r1, const vf32 & r2, const vf32 & r3)
	{
		r[0] = r0;
		r[1] = r1;
		r[2] = r2;
		r[3] = r3;
	}

	vf32 r[4];
};

using qf32 = vf32;
using crqf32 = const qf32&;

#define CRBvVec const BvVec&
#define CRBvQuat const BvQuat&

#endif
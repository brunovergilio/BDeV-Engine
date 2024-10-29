#pragma once


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_AVX)


BV_INLINE vf32 BV_VCALL Load(const f32* p)
{
	return _mm_load_ss(p);
}

BV_INLINE vf32 BV_VCALL Load2(const f32* p)
{
	return _mm_castpd_ps(_mm_load_sd((const double*)p));
}

BV_INLINE vf32 BV_VCALL Load3(const f32* p)
{
	__m128 xy = _mm_castpd_ps(_mm_load_sd((const double*)p));
	__m128 z = _mm_load_ss(&p[2]);
	return _mm_movelh_ps(xy, z);
}

BV_INLINE vf32 BV_VCALL Load4(const f32* p)
{
	return _mm_loadu_ps(p);
}

BV_INLINE vf32 BV_VCALL Load4A(const f32* p)
{
	return _mm_load_ps(p);
}

BV_INLINE void BV_VCALL Store(cvf32 v, f32* p)
{
	_mm_store_ss(p, v);
}

BV_INLINE void BV_VCALL Store2(cvf32 v, f32* p)
{
	_mm_store_sd((double*)p, _mm_castps_pd(v));
}

BV_INLINE void BV_VCALL Store3(cvf32 v, f32* p)
{
	_mm_store_sd((double*)p, _mm_castps_pd(v));
	vf32 z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
	_mm_store_ss(&p[2], z);
}

BV_INLINE void BV_VCALL Store4(cvf32 v, f32* p)
{
	_mm_storeu_ps(p, v);
}

BV_INLINE void BV_VCALL Store4A(cvf32 v, f32* p)
{
	_mm_store_ps(p, v);
}

BV_INLINE mf32 BV_VCALL Load22(const f32* p)
{
	mf32 r;
	r.r[0] = Load2(p);
	r.r[1] = Load2(p + 2);
	r.r[2] = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

BV_INLINE mf32 BV_VCALL Load33(const f32* p)
{
	mf32 r;
	r.r[0] = Load3(p);
	r.r[1] = Load3(p + 3);
	r.r[2] = Load3(p + 6);
	r.r[3] = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

BV_INLINE mf32 BV_VCALL Load43(const f32* p)
{
	mf32 r;
	r.r[0] = Load3(p);
	r.r[1] = Load3(p + 3);
	r.r[2] = Load3(p + 6);
	r.r[3] = Load3(p + 9);

	return r;
}

BV_INLINE mf32 BV_VCALL Load44(const f32* p)
{
	mf32 r;
	r.r[0] = Load4(p);
	r.r[1] = Load4(p + 4);
	r.r[2] = Load4(p + 8);
	r.r[3] = Load4(p + 12);

	return r;
}

BV_INLINE mf32 BV_VCALL Load44A(const f32* p)
{
	mf32 r;
	r.r[0] = Load4A(p);
	r.r[1] = Load4A(p + 4);
	r.r[2] = Load4A(p + 8);
	r.r[3] = Load4A(p + 12);

	return r;
}

BV_INLINE void BV_VCALL Store22(const mf32& m, f32* p)
{
	Store2(m.r[0], p);
	Store2(m.r[1], p + 2);
}

BV_INLINE void BV_VCALL Store33(const mf32& m, f32* p)
{
	Store3(m.r[0], p);
	Store3(m.r[1], p + 3);
	Store3(m.r[2], p + 6);
}

BV_INLINE void BV_VCALL Store43(const mf32& m, f32* p)
{
	Store3(m.r[0], p);
	Store3(m.r[1], p + 3);
	Store3(m.r[2], p + 6);
	Store3(m.r[3], p + 9);
}

BV_INLINE void BV_VCALL Store44(const mf32& m, f32* p)
{
	Store4(m.r[0], p);
	Store4(m.r[1], p + 4);
	Store4(m.r[2], p + 8);
	Store4(m.r[3], p + 12);
}

BV_INLINE void BV_VCALL Store44A(const mf32& m, f32* p)
{
	Store4A(m.r[0], p);
	Store4A(m.r[1], p + 4);
	Store4A(m.r[2], p + 8);
	Store4A(m.r[3], p + 12);
}

#else

BV_INLINE vf32 BV_VCALL Load(const f32* p)
{
	return vf32(p[0], 0.0f, 0.0f, 0.0f);
}

BV_INLINE vf32 BV_VCALL Load2(const f32* p)
{
	return vf32(p[0], p[1], 0.0f, 0.0f);
}

BV_INLINE vf32 BV_VCALL Load3(const f32* p)
{
	return vf32(p[0], p[1], p[2], 0.0f);
}

BV_INLINE vf32 BV_VCALL Load4(const f32* p)
{
	return vf32(p[0], p[1], p[2], p[3]);
}

BV_INLINE vf32 BV_VCALL Load4A(const f32* p)
{
	return vf32(p[0], p[1], p[2], p[3]);
}

BV_INLINE void BV_VCALL Store(crvf32 v, f32* p)
{
	p[0] = v.x;
}

BV_INLINE void BV_VCALL Store2(crvf32 v, f32* p)
{
	p[0] = v.x;
	p[1] = v.y;
}

BV_INLINE void BV_VCALL Store3(crvf32 v, f32* p)
{
	p[0] = v.x;
	p[1] = v.y;
	p[2] = v.z;
}

BV_INLINE void BV_VCALL Store4(crvf32 v, f32* p)
{
	p[0] = v.x;
	p[1] = v.y;
	p[2] = v.z;
	p[3] = v.w;
}

BV_INLINE void BV_VCALL Store4A(crvf32 v, f32* p)
{
	p[0] = v.x;
	p[1] = v.y;
	p[2] = v.z;
	p[3] = v.w;
}

BV_INLINE mf32 BV_VCALL Load22(const f32* p)
{
	mf32 r;
	r.r[0] = Load2(p);
	r.r[1] = Load2(p + 2);
	r.r[2].Set(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3].Set(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

BV_INLINE mf32 BV_VCALL Load33(const f32* p)
{
	mf32 r;
	r.r[0] = Load3(p);
	r.r[1] = Load3(p + 3);
	r.r[2] = Load3(p + 6);
	r.r[3].Set(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

BV_INLINE mf32 BV_VCALL Load43(const f32* p)
{
	mf32 r;
	r.r[0] = Load3(p);
	r.r[1] = Load3(p + 3);
	r.r[2] = Load3(p + 6);
	r.r[3] = Load3(p + 9);

	return r;
}

BV_INLINE mf32 BV_VCALL Load44(const f32* p)
{
	mf32 r;
	r.r[0] = Load4(p);
	r.r[1] = Load4(p + 4);
	r.r[2] = Load4(p + 8);
	r.r[3] = Load4(p + 12);

	return r;
}

BV_INLINE mf32 BV_VCALL Load44A(const f32* p)
{
	mf32 r;
	r.r[0] = Load4A(p);
	r.r[1] = Load4A(p + 4);
	r.r[2] = Load4A(p + 8);
	r.r[3] = Load4A(p + 12);

	return r;
}

BV_INLINE void BV_VCALL Store22(const mf32& m, f32* p)
{
	Store2(m.r[0], p);
	Store2(m.r[1], p + 2);
}

BV_INLINE void BV_VCALL Store33(const mf32& m, f32* p)
{
	Store3(m.r[0], p);
	Store3(m.r[1], p + 3);
	Store3(m.r[2], p + 6);
}

BV_INLINE void BV_VCALL Store43(const mf32& m, f32* p)
{
	Store3(m.r[0], p);
	Store3(m.r[1], p + 3);
	Store3(m.r[2], p + 6);
	Store3(m.r[3], p + 9);
}

BV_INLINE void BV_VCALL Store44(const mf32& m, f32* p)
{
	Store4(m.r[0], p);
	Store4(m.r[1], p + 4);
	Store4(m.r[2], p + 8);
	Store4(m.r[3], p + 12);
}

BV_INLINE void BV_VCALL Store44A(const mf32& m, f32* p)
{
	Store4A(m.r[0], p);
	Store4A(m.r[1], p + 4);
	Store4A(m.r[2], p + 8);
	Store4A(m.r[3], p + 12);
}


#endif
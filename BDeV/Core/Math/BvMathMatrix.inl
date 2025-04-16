#pragma once


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_AVX)

BV_INLINE mf32 BV_VCALL MatrixSet(cvf32 v0, cvf32 v1, cvf32 v2, cvf32 v3)
{
	mf32 r;
	r.r[0] = v0;
	r.r[1] = v1;
	r.r[2] = v2;
	r.r[3] = v3;

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixAdd(cmf32 m1, cmf32 m2)
{
	mf32 m;
	m.r[0] = _mm_add_ps(m1.r[0], m2.r[0]);
	m.r[1] = _mm_add_ps(m1.r[1], m2.r[1]);
	m.r[2] = _mm_add_ps(m1.r[2], m2.r[2]);
	m.r[3] = _mm_add_ps(m1.r[3], m2.r[3]);

	return m;
}

BV_INLINE mf32 BV_VCALL MatrixAdd(cmf32 m, f32 s)
{
	vf32 v = _mm_set1_ps(s);
	mf32 r;
	r.r[0] = _mm_add_ps(m.r[0], v);
	r.r[1] = _mm_add_ps(m.r[1], v);
	r.r[2] = _mm_add_ps(m.r[2], v);
	r.r[3] = _mm_add_ps(m.r[3], v);

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixSub(cmf32 m1, cmf32 m2)
{
	mf32 m;
	m.r[0] = _mm_sub_ps(m1.r[0], m2.r[0]);
	m.r[1] = _mm_sub_ps(m1.r[1], m2.r[1]);
	m.r[2] = _mm_sub_ps(m1.r[2], m2.r[2]);
	m.r[3] = _mm_sub_ps(m1.r[3], m2.r[3]);

	return m;
}

BV_INLINE mf32 BV_VCALL MatrixSub(cmf32 m, f32 s)
{
	vf32 v = _mm_set1_ps(s);
	mf32 r;
	r.r[0] = _mm_sub_ps(m.r[0], v);
	r.r[1] = _mm_sub_ps(m.r[1], v);
	r.r[2] = _mm_sub_ps(m.r[2], v);
	r.r[3] = _mm_sub_ps(m.r[3], v);

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixMul(cmf32 m1, cmf32 m2)
{
	mf32 m;

	// Row 0
	vf32 tmp0 = _mm_shuffle_ps(m1.r[0], m1.r[0], _MM_SHUFFLE(0, 0, 0, 0));
	tmp0 = _mm_mul_ps(tmp0, m2.r[0]);

	vf32 tmp1 = _mm_shuffle_ps(m1.r[0], m1.r[0], _MM_SHUFFLE(1, 1, 1, 1));
	tmp1 = _mm_mul_ps(tmp1, m2.r[1]);

	vf32 tmp2 = _mm_shuffle_ps(m1.r[0], m1.r[0], _MM_SHUFFLE(2, 2, 2, 2));
	tmp2 = _mm_mul_ps(tmp2, m2.r[2]);

	vf32 tmp3 = _mm_shuffle_ps(m1.r[0], m1.r[0], _MM_SHUFFLE(3, 3, 3, 3));
	tmp3 = _mm_mul_ps(tmp3, m2.r[3]);

	m.r[0] = _mm_add_ps(tmp0, tmp1);
	m.r[0] = _mm_add_ps(m.r[0], _mm_add_ps(tmp2, tmp3));

	// Row 1
	tmp0 = _mm_shuffle_ps(m1.r[1], m1.r[1], _MM_SHUFFLE(0, 0, 0, 0));
	tmp0 = _mm_mul_ps(tmp0, m2.r[0]);

	tmp1 = _mm_shuffle_ps(m1.r[1], m1.r[1], _MM_SHUFFLE(1, 1, 1, 1));
	tmp1 = _mm_mul_ps(tmp1, m2.r[1]);

	tmp2 = _mm_shuffle_ps(m1.r[1], m1.r[1], _MM_SHUFFLE(2, 2, 2, 2));
	tmp2 = _mm_mul_ps(tmp2, m2.r[2]);

	tmp3 = _mm_shuffle_ps(m1.r[1], m1.r[1], _MM_SHUFFLE(3, 3, 3, 3));
	tmp3 = _mm_mul_ps(tmp3, m2.r[3]);

	m.r[1] = _mm_add_ps(tmp0, tmp1);
	m.r[1] = _mm_add_ps(m.r[1], _mm_add_ps(tmp2, tmp3));

	// Row 2
	tmp0 = _mm_shuffle_ps(m1.r[2], m1.r[2], _MM_SHUFFLE(0, 0, 0, 0));
	tmp0 = _mm_mul_ps(tmp0, m2.r[0]);

	tmp1 = _mm_shuffle_ps(m1.r[2], m1.r[2], _MM_SHUFFLE(1, 1, 1, 1));
	tmp1 = _mm_mul_ps(tmp1, m2.r[1]);

	tmp2 = _mm_shuffle_ps(m1.r[2], m1.r[2], _MM_SHUFFLE(2, 2, 2, 2));
	tmp2 = _mm_mul_ps(tmp2, m2.r[2]);

	tmp3 = _mm_shuffle_ps(m1.r[2], m1.r[2], _MM_SHUFFLE(3, 3, 3, 3));
	tmp3 = _mm_mul_ps(tmp3, m2.r[3]);

	m.r[2] = _mm_add_ps(tmp0, tmp1);
	m.r[2] = _mm_add_ps(m.r[2], _mm_add_ps(tmp2, tmp3));

	// Row 3
	tmp0 = _mm_shuffle_ps(m1.r[3], m1.r[3], _MM_SHUFFLE(0, 0, 0, 0));
	tmp0 = _mm_mul_ps(tmp0, m2.r[0]);

	tmp1 = _mm_shuffle_ps(m1.r[3], m1.r[3], _MM_SHUFFLE(1, 1, 1, 1));
	tmp1 = _mm_mul_ps(tmp1, m2.r[1]);

	tmp2 = _mm_shuffle_ps(m1.r[3], m1.r[3], _MM_SHUFFLE(2, 2, 2, 2));
	tmp2 = _mm_mul_ps(tmp2, m2.r[2]);

	tmp3 = _mm_shuffle_ps(m1.r[3], m1.r[3], _MM_SHUFFLE(3, 3, 3, 3));
	tmp3 = _mm_mul_ps(tmp3, m2.r[3]);

	m.r[3] = _mm_add_ps(tmp0, tmp1);
	m.r[3] = _mm_add_ps(m.r[3], _mm_add_ps(tmp2, tmp3));

	return m;
}

BV_INLINE mf32 BV_VCALL MatrixMul(cmf32 m, f32 s)
{
	vf32 v = _mm_set1_ps(s);
	mf32 r;
	r.r[0] = _mm_mul_ps(m.r[0], v);
	r.r[1] = _mm_mul_ps(m.r[1], v);
	r.r[2] = _mm_mul_ps(m.r[2], v);
	r.r[3] = _mm_mul_ps(m.r[3], v);

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixDiv(cmf32 m, f32 s)
{
	vf32 v = _mm_set1_ps(s);
	mf32 r;
	r.r[0] = _mm_div_ps(m.r[0], v);
	r.r[1] = _mm_div_ps(m.r[1], v);
	r.r[2] = _mm_div_ps(m.r[2], v);
	r.r[3] = _mm_div_ps(m.r[3], v);

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixIdentity()
{
	vf32 id = _mm_set_ss(1.0f);

	mf32 m;
	m.r[0] = id;
	m.r[1] = _mm_shuffle_ps(id, id, _MM_SHUFFLE(1, 1, 0, 1));
	m.r[2] = _mm_shuffle_ps(id, id, _MM_SHUFFLE(1, 0, 1, 1));
	m.r[3] = _mm_shuffle_ps(id, id, _MM_SHUFFLE(0, 1, 1, 1));

	return m;
}

BV_INLINE mf32 BV_VCALL MatrixTranspose(cmf32 m)
{
	// top left 2x2 matrix
	vf32 upperBlock = _mm_shuffle_ps(m.r[0], m.r[1], _MM_SHUFFLE(1, 0, 1, 0));
	// bottom left 2x2 matrix
	vf32 lowerBlock = _mm_shuffle_ps(m.r[2], m.r[3], _MM_SHUFFLE(1, 0, 1, 0));

	mf32 mT;
	mT.r[0] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(2, 0, 2, 0));
	mT.r[1] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(3, 1, 3, 1));

	// top right 2x2 matrix
	upperBlock = _mm_shuffle_ps(m.r[0], m.r[1], _MM_SHUFFLE(3, 2, 3, 2));
	// bottom right 2x2 matrix
	lowerBlock = _mm_shuffle_ps(m.r[2], m.r[3], _MM_SHUFFLE(3, 2, 3, 2));

	mT.r[2] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(2, 0, 2, 0));
	mT.r[3] = _mm_shuffle_ps(upperBlock, lowerBlock, _MM_SHUFFLE(3, 1, 3, 1));

	return mT;
}

BV_INLINE vf32 BV_VCALL MatrixDeterminant(cmf32 m)
{
	// Calculate the first 12 2x2 determinant multiplications, to avoid repeating them later
	vf32 cf1 = _mm_mul_ps(_mm_shuffle_ps(m.r[2], m.r[2], _MM_SHUFFLE(3, 1, 3, 2)), _mm_shuffle_ps(m.r[3], m.r[3], _MM_SHUFFLE(1, 3, 2, 3)));
	vf32 cf2 = _mm_mul_ps(_mm_shuffle_ps(m.r[2], m.r[2], _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(m.r[3], m.r[3], _MM_SHUFFLE(0, 3, 1, 2)));
	vf32 cf3 = _mm_mul_ps(_mm_shuffle_ps(m.r[2], m.r[2], _MM_SHUFFLE(1, 0, 2, 0)), _mm_shuffle_ps(m.r[3], m.r[3], _MM_SHUFFLE(0, 1, 0, 2)));

	// Compute the first multiplication for the first row
	vf32 tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(3, 2, 1, 0));
	vf32 tmp2 = _mm_shuffle_ps(m.r[1], m.r[1], _MM_SHUFFLE(2, 3, 0, 1));

	vf32 row0 = _mm_mul_ps(tmp1, tmp2);

	// Compute the second multiplication and first subtraction for the first row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));

	row0 = _mm_sub_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the third multiplication and first addition for the first row
	tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(0, 1, 2, 3));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(3, 1, 2, 0));
	tmp2 = _mm_shuffle_ps(m.r[1], m.r[1], _MM_SHUFFLE(1, 0, 3, 2));

	row0 = _mm_add_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the fourth multiplication and second subtraction for the first row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2));

	row0 = _mm_sub_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the fifth multiplication and second addition for the first row
	tmp1 = _mm_shuffle_ps(cf2, cf2, _MM_SHUFFLE(1, 3, 2, 0));
	tmp2 = _mm_shuffle_ps(m.r[1], m.r[1], _MM_SHUFFLE(0, 1, 2, 3));

	row0 = _mm_add_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the sixth multiplication and third subtraction for the first row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(0, 1, 2, 3));

	row0 = _mm_sub_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the determinant
	vf32 det = _mm_mul_ps(row0, m.r[0]);

	// Sum all values and replicate them through all elements
	det = _mm_add_ps(det, _mm_shuffle_ps(det, det, _MM_SHUFFLE(2, 3, 0, 1)));
	det = _mm_add_ps(det, _mm_shuffle_ps(det, det, _MM_SHUFFLE(1, 0, 3, 2)));

	return det;
}


BV_INLINE mf32 BV_VCALL MatrixInverse(cmf32 m, vf32& d)
{
	mf32 mTmp = MatrixTranspose(m);

	// Calculate the first 12 2x2 determinant multiplications, to avoid repeating them later
	vf32 cf1 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(3, 1, 3, 2)), _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(1, 3, 2, 3)));
	vf32 cf2 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(0, 3, 1, 2)));
	vf32 cf3 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(1, 0, 2, 0)), _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(0, 1, 0, 2)));

	// Compute the first multiplication for the first row
	vf32 tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(3, 2, 1, 0));
	vf32 tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(2, 3, 0, 1));

	vf32 row0 = _mm_mul_ps(tmp1, tmp2);

	// Compute the first multiplication for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(2, 3, 0, 1));
	vf32 row1 = _mm_mul_ps(tmp1, tmp2);

	// Compute the second multiplication and first subtraction for the first row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));
	tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(2, 3, 0, 1));

	row0 = _mm_sub_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the second multiplication and first subtraction for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(2, 3, 0, 1));

	row1 = _mm_sub_ps(_mm_mul_ps(tmp1, tmp2), row1);

	// Compute the third multiplication and first addition for the first row
	tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(0, 1, 2, 3));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(3, 1, 2, 0));
	tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(1, 0, 3, 2));

	row0 = _mm_add_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the third multiplication and second subtraction for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(1, 0, 3, 2));

	row1 = _mm_sub_ps(row1, _mm_mul_ps(tmp1, tmp2));

	// Compute the fourth multiplication and second subtraction for the first row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2));
	tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(1, 0, 3, 2));

	row0 = _mm_sub_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the fourth multiplication and first addition for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(1, 0, 3, 2));

	row1 = _mm_add_ps(_mm_mul_ps(tmp1, tmp2), row1);

	// Compute the fifth multiplication and second addition for the first row
	tmp1 = _mm_shuffle_ps(cf2, cf2, _MM_SHUFFLE(1, 3, 2, 0));
	tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(0, 1, 2, 3));

	row0 = _mm_add_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the fifth multiplication and third subtraction for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(0, 1, 2, 3));

	row1 = _mm_sub_ps(row1, _mm_mul_ps(tmp1, tmp2));

	// Compute the sixth multiplication and third subtraction for the first row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(0, 1, 2, 3));
	tmp2 = _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(0, 1, 2, 3));

	row0 = _mm_sub_ps(row0, _mm_mul_ps(tmp1, tmp2));

	// Compute the sixth multiplication and second addition for the second row
	tmp2 = _mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(0, 1, 2, 3));

	row1 = _mm_add_ps(row1, _mm_mul_ps(tmp1, tmp2));

	// Calculate the second 12 2x2 determinant multiplications, avoid repetitions
	cf1 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(3, 1, 3, 2)), _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(1, 3, 2, 3)));
	cf2 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(0, 3, 1, 2)));
	cf3 = _mm_mul_ps(_mm_shuffle_ps(mTmp.r[0], mTmp.r[0], _MM_SHUFFLE(1, 0, 2, 0)), _mm_shuffle_ps(mTmp.r[1], mTmp.r[1], _MM_SHUFFLE(0, 1, 0, 2)));

	// Compute the first multiplication for the third row
	tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(3, 2, 1, 0));
	tmp2 = _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(2, 3, 0, 1));

	vf32 row2 = _mm_mul_ps(tmp1, tmp2);

	// Compute the first multiplication for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(2, 3, 0, 1));
	vf32 row3 = _mm_mul_ps(tmp1, tmp2);

	// Compute the second multiplication and first subtraction for the third row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1));
	tmp2 = _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(2, 3, 0, 1));

	row2 = _mm_sub_ps(row2, _mm_mul_ps(tmp1, tmp2));

	// Compute the second multiplication and first subtraction for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(2, 3, 0, 1));

	row3 = _mm_sub_ps(_mm_mul_ps(tmp1, tmp2), row3);

	// Compute the third multiplication and first addition for the third row
	tmp1 = _mm_shuffle_ps(cf1, cf3, _MM_SHUFFLE(0, 1, 2, 3));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(3, 1, 2, 0));
	tmp2 = _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(1, 0, 3, 2));

	row2 = _mm_add_ps(row2, _mm_mul_ps(tmp1, tmp2));

	// Compute the third multiplication and second subtraction for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(1, 0, 3, 2));

	row3 = _mm_sub_ps(row3, _mm_mul_ps(tmp1, tmp2));

	// Compute the fourth multiplication and second subtraction for the third row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2));
	tmp2 = _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(1, 0, 3, 2));

	row2 = _mm_sub_ps(row2, _mm_mul_ps(tmp1, tmp2));

	// Compute the fourth multiplication and first addition for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(1, 0, 3, 2));

	row3 = _mm_add_ps(_mm_mul_ps(tmp1, tmp2), row3);

	// Compute the fifth multiplication and second addition for the third row
	tmp1 = _mm_shuffle_ps(cf2, cf2, _MM_SHUFFLE(1, 3, 2, 0));
	tmp2 = _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(0, 1, 2, 3));

	row2 = _mm_add_ps(row2, _mm_mul_ps(tmp1, tmp2));

	// Compute the fifth multiplication and third subtraction for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(0, 1, 2, 3));

	row3 = _mm_sub_ps(row3, _mm_mul_ps(tmp1, tmp2));

	// Compute the sixth multiplication and third subtraction for the third row
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(0, 1, 2, 3));
	tmp2 = _mm_shuffle_ps(mTmp.r[3], mTmp.r[3], _MM_SHUFFLE(0, 1, 2, 3));

	row2 = _mm_sub_ps(row2, _mm_mul_ps(tmp1, tmp2));

	// Compute the sixth multiplication and second addition for the fourth row
	tmp2 = _mm_shuffle_ps(mTmp.r[2], mTmp.r[2], _MM_SHUFFLE(0, 1, 2, 3));

	row3 = _mm_add_ps(row3, _mm_mul_ps(tmp1, tmp2));

	// Compute the determinant
	vf32 det = _mm_mul_ps(row0, mTmp.r[0]);

	// Sum all values and replicate them through all elements
	det = _mm_add_ps(det, _mm_shuffle_ps(det, det, _MM_SHUFFLE(2, 3, 0, 1)));
	det = _mm_add_ps(det, _mm_shuffle_ps(det, det, _MM_SHUFFLE(1, 0, 3, 2)));

	d = det;

	// Get 1/Det
	det = _mm_rcp_ps(det); // _mm_rcp_ps is faster but less precise
	//det = _mm_div_ps(_mm_set1_ps(1.0f), det); // _mm_div_ps is not as fast as _mm_rcp_ps but is more precise

	// Multiply every element by 1/Det
	mTmp.r[0] = _mm_mul_ps(row0, det);
	mTmp.r[1] = _mm_mul_ps(row1, det);
	mTmp.r[2] = _mm_mul_ps(row2, det);
	mTmp.r[3] = _mm_mul_ps(row3, det);

	return mTmp;
}

BV_INLINE mf32 BV_VCALL MatrixInverse(cmf32 m)
{
	vf32 d;
	return MatrixInverse(m, d);
}

BV_INLINE mf32 BV_VCALL MatrixScaling(cvf32 s)
{
	vf32 v1 = VectorUnitW();

	mf32 r;
	r.r[0] = VectorBlend<0, 4, 5, 6>(s, v1);
	r.r[1] = VectorBlend<4, 1, 5, 6>(s, v1);
	r.r[2] = VectorBlend<4, 5, 2, 6>(s, v1);
	r.r[3] = v1;

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixScaling(f32 sX, f32 sY, f32 sZ)
{
	mf32 r;
	r.r[0] = VectorSet(sX, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, sY, 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, sZ, 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixTranslation(cvf32 t)
{
	mf32 r;
	r.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = VectorSetW(t, 1.0f);

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixTranslation(f32 tX, f32 tY, f32 tZ)
{
	mf32 r;
	r.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = VectorSet(tX, tY, tZ, 1.0f);

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixRotationX(f32 angle)
{
	f32 cosSin[] = { cosf(angle), sinf(angle) };
	vf32 vCosSin = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);
	cosSin[1] = -cosSin[1];
	vf32 vMinusSinCos = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);

	mf32 r;
	r.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	r.r[1] = _mm_shuffle_ps(vCosSin, vCosSin, _MM_SHUFFLE(3, 1, 0, 2));
	r.r[2] = _mm_shuffle_ps(vMinusSinCos, vMinusSinCos, _MM_SHUFFLE(3, 0, 1, 2));
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixRotationY(f32 angle)
{
	f32 cosSin[] = { cosf(angle), sinf(angle) };
	vf32 vCosSin = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);
	cosSin[1] = -cosSin[1];
	vf32 vMinusSinCos = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(vMinusSinCos, vMinusSinCos, _MM_SHUFFLE(3, 1, 2, 0));
	r.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	r.r[2] = _mm_shuffle_ps(vCosSin, vCosSin, _MM_SHUFFLE(3, 0, 2, 1));
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixRotationZ(f32 angle)
{
	f32 cosSin[] = { cosf(angle), sinf(angle) };
	vf32 vCosSin = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);
	cosSin[1] = -cosSin[1];
	vf32 vMinusSinCos = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);

	mf32 r;
	r.r[0] = vCosSin;
	r.r[1] = _mm_shuffle_ps(vMinusSinCos, vMinusSinCos, _MM_SHUFFLE(3, 2, 0, 1));
	r.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixRotationAxis(cvf32 axis, f32 angle)
{
	f32 sin = sinf(angle);
	f32 cos = cosf(angle);

	vf32 vSin = _mm_set1_ps(sin);
	vf32 vCos = _mm_set_ps(0.0f, cos, cos, cos);
	vf32 vOneMinusCos = _mm_set1_ps(1.0f - cos);

	// x * (1 - cos), y * (1 - cos), z * (1 - cos)
	vf32 d = _mm_mul_ps(axis, vOneMinusCos);

	vf32 t1 = d;

	// x^2 * (1 - cos), y^2 * (1 - cos), z^2 * (1 - cos)
	d = _mm_mul_ps(d, axis);
	// cos + x^2 * (1 - cos), cos + y^2 * (1 - cos), cos + z^2 * (1 - cos)
	d = _mm_add_ps(d, vCos);

	// x * (sin), y * (sin), z * (sin)
	vf32 t0 = _mm_mul_ps(axis, vSin);

	// z * (1 - cos), x * (1 - cos), y * (1 - cos)
	t1 = _mm_shuffle_ps(t1, t1, _MM_SHUFFLE(3, 1, 0, 2));
	// y * z * (1 - cos), z * x * (1 - cos), x * y * (1 - cos)
	t1 = _mm_mul_ps(t1, _mm_shuffle_ps(axis, axis, _MM_SHUFFLE(3, 0, 2, 1)));
	// y * z * (1 - cos) - x * (sin), z * x * (1 - cos) - y * (sin), x * y * (1 - cos) - z * (sin)
	vf32 t2 = _mm_sub_ps(t1, t0);
	// y * z * (1 - cos) + x * (sin), z * x * (1 - cos) + y * (sin), x * y * (1 - cos) + z * (sin)
	t1 = _mm_add_ps(t1, t0);

	// y * z * (1 - cos) + x * (sin), x * y * (1 - cos) + z * (sin), z * x * (1 - cos) - y * (sin), x * y * (1 - cos) - z * (sin)
	vf32 g0 = _mm_shuffle_ps(t1, t2, _MM_SHUFFLE(2, 1, 2, 0));
	// y * z * (1 - cos) + x * (sin), 0, y * z * (1 - cos) - x * (sin), 0
	vf32 g1 = _mm_shuffle_ps(t1, t2, _MM_SHUFFLE(3, 0, 3, 1));

	mf32 r;
	r.r[0] = _mm_shuffle_ps(d, g0, _MM_SHUFFLE(2, 1, 3, 0));
	r.r[0] = _mm_shuffle_ps(r.r[0], r.r[0], _MM_SHUFFLE(1, 3, 2, 0));

	r.r[1] = _mm_shuffle_ps(d, g0, _MM_SHUFFLE(0, 3, 3, 1));
	r.r[1] = _mm_shuffle_ps(r.r[1], r.r[1], _MM_SHUFFLE(1, 3, 0, 2));

	r.r[2] = _mm_shuffle_ps(d, g1, _MM_SHUFFLE(2, 0, 3, 2));
	r.r[2] = _mm_shuffle_ps(r.r[2], r.r[2], _MM_SHUFFLE(1, 0, 3, 2));

	r.r[3] = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixFromQuaternion(cvf32 q)
{
	// v0 = 2 * q.x, 2 * q.y, 2 * q.z, 2 * q.w
	vf32 v0 = _mm_add_ps(q, q);

	// v1 = 2 * q.x, 2 * q.x, 2 * q.x, 2 * q.y
	vf32 v1 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(1, 0, 0, 0));
	// v3 = q.y, q.z, q.y, q.z
	vf32 v3 = _mm_shuffle_ps(q, q, _MM_SHUFFLE(2, 1, 2, 1));
	// v1 = 2 * q.x * q.y, 2 * q.x * q.z, 2 * q.x * q.y, 2 * q.y * q.z
	v1 = _mm_mul_ps(v1, v3);

	// v2 = 2 * q.w, 2 * q.w, 2 * q.w, 2 * q.w
	vf32 v2 = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 3));
	// v3 = q.z, q.y, q.z, q.x
	v3 = _mm_shuffle_ps(q, q, _MM_SHUFFLE(0, 2, 1, 2));
	// v2 = 2 * q.w * q.z, 2 * q.w * q.y, 2 * q.w * q.z, 2 * q.w * q.x
	v2 = _mm_mul_ps(v2, v3);


	// v3 = 2 * q.x * q.y + 2 * q.w * q.z
	//      2 * q.x * q.z + 2 * q.w * q.y
	//      2 * q.x * q.y + 2 * q.w * q.z
	//      2 * q.y * q.z + 2 * q.w * q.x
	v3 = _mm_add_ps(v1, v2);

	// v2 = 2 * q.x * q.y - 2 * q.w * q.z
	//      2 * q.x * q.z - 2 * q.w * q.y
	//      2 * q.x * q.y - 2 * q.w * q.z
	//      2 * q.y * q.z - 2 * q.w * q.x
	v2 = _mm_sub_ps(v1, v2);

	// v1 = 2 * q.x * q.y + 2 * q.w * q.z
	//      2 * q.y * q.z + 2 * q.w * q.x
	//      2 * q.x * q.z - 2 * q.w * q.y
	//      2 * q.x * q.y - 2 * q.w * q.z
	v1 = _mm_shuffle_ps(v3, v2, _MM_SHUFFLE(0, 1, 3, 0));

	// v2 = 2 * q.x * q.z + 2 * q.w * q.y
	//      2 * q.x * q.z + 2 * q.w * q.y
	//      2 * q.y * q.z - 2 * q.w * q.x
	//      2 * q.y * q.z - 2 * q.w * q.x
	v2 = _mm_shuffle_ps(v3, v2, _MM_SHUFFLE(3, 3, 1, 1));

	// c0 = 1, 1, 1, 0
	vf32 c0 = _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f);

	// v0 = 2 * q.x ^ 2, 2 * q.y ^ 2, 2 * q.z ^ 2, 2 * q.w ^ 2
	v0 = _mm_mul_ps(v0, q);
	// v0 = 2 * q.x ^ 2, 2 * q.y ^ 2, 2 * q.z ^ 2, 0
	v0 = _mm_mul_ps(c0, v0);
	// v0 = 2 * q.x ^ 2 + 2 * q.y ^ 2, 2 * q.y ^ 2 + 2 * q.z ^ 2, 2 * q.z ^ 2 + 2 * q.x ^ 2, 0
	v0 = _mm_add_ps(v0, _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 0, 2, 1)));
	// v0 = 1 - 2 * q.x ^ 2 - 2 * q.y ^ 2, 1 - 2 * q.y ^ 2 - 2 * q.z ^ 2, 1 - 2 * q.z ^ 2 - 2 * q.x ^ 2, 0
	v0 = _mm_sub_ps(c0, v0);

	// v3 = 1 - 2 * q.y ^ 2 - 2 * q.z ^ 2, 0, 2 * q.x * q.y + 2 * q.w * q.z, 2 * q.x * q.z - 2 * q.w * q.y
	v3 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(2, 0, 3, 1));
	// v3 = 1 - 2 * q.y ^ 2 - 2 * q.z ^ 2, 2 * q.x * q.y + 2 * q.w * q.z, 2 * q.x * q.z - 2 * q.w * q.y, 0
	v3 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(1, 3, 2, 0));

	mf32 r;
	r.r[0] = v3;

	// v3 = 1 - 2 * q.z ^ 2 - 2 * q.x ^ 2, 0, 2 * q.x * q.y - 2 * q.w * q.z, 2 * q.y * q.z + 2 * q.w * q.x
	v3 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(1, 3, 3, 2));
	// v3 = 2 * q.x * q.y - 2 * q.w * q.z, 1 - 2 * q.z ^ 2 - 2 * q.x ^ 2, 2 * q.y * q.z + 2 * q.w * q.x, 0
	v3 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(1, 3, 0, 2));

	r.r[1] = v3;

	// v3 = 1 - 2 * q.x ^ 2 - 2 * q.y ^ 2, 0, 2 * q.x * q.z + 2 * q.w * q.y, 2 * q.y * q.z - 2 * q.w * q.x
	v3 = _mm_shuffle_ps(v0, v2, _MM_SHUFFLE(2, 0, 3, 0));
	// v3 = 2 * q.x * q.z + 2 * q.w * q.y, 2 * q.y * q.z - 2 * q.w * q.x, 1 - 2 * q.x ^ 2 - 2 * q.y ^ 2, 0
	v3 = _mm_shuffle_ps(v3, v3, _MM_SHUFFLE(1, 0, 3, 2));

	r.r[2] = v3;

	// c0 = 0, 0, 0, 1
	c0 = _mm_shuffle_ps(c0, c0, _MM_SHUFFLE(0, 3, 3, 3));

	r.r[3] = c0;

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixLookAt(cvf32 eyePos, cvf32 dirVec, cvf32 upVec)
{
	vf32 z = Vector3Normalize(dirVec);

	vf32 x = Vector3Cross(upVec, z);
	x = Vector3Normalize(x);

	vf32 y = Vector3Cross(z, x);

	mf32 r;
	r.r[0] = x;
	r.r[1] = y;
	r.r[2] = z;
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	r = MatrixTranspose(r);
	{
		vf32 d = Vector3Dot(x, eyePos);
		d = VectorBlend<0, 5, 2, 3>(d, Vector3Dot(y, eyePos));
		d = VectorBlend<0, 1, 6, 3>(d, Vector3Dot(z, eyePos));
		d = VectorSetW(d, 1.0f);
		r.r[3] = VectorChangeSign<1, 1, 1, 0>(d);
	}

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixLookAtLH(cvf32 eyePos, cvf32 lookPos, cvf32 upVec)
{
	vf32 dir = VectorSub(lookPos, eyePos);
	return MatrixLookAt(eyePos, dir, upVec);
}

BV_INLINE mf32 BV_VCALL MatrixLookAtRH(cvf32 eyePos, cvf32 lookPos, cvf32 upVec)
{
	vf32 dir = VectorSub(eyePos, lookPos);
	return MatrixLookAt(eyePos, dir, upVec);
}

BV_INLINE mf32 BV_VCALL MatrixPerspectiveLH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ / (farZ - nearZ);

	vf32 v0 = _mm_set_ps(0.0f, -nearZ * b, a, a / aspectRatio);
	vf32 v1 = _mm_set_ps(0.0f, 0.0f, 1.0f, b);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixPerspectiveRH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ / (nearZ - farZ);

	vf32 v0 = _mm_set_ps(0.0f, nearZ * b, a, a / aspectRatio);
	vf32 v1 = _mm_set_ps(0.0f, 0.0f, -1.0f, b);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixPerspectiveLH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = 1.0f / (farZ - nearZ);

	vf32 v0 = _mm_set_ps(0.0f, -2.0f * farZ * nearZ * b, a, a / aspectRatio);
	vf32 v1 = _mm_set_ps(0.0f, 0.0f, 1.0f, (farZ + nearZ) * b);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixPerspectiveRH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = 1.0f / (nearZ - farZ);

	vf32 v0 = _mm_set_ps(0.0f, 2.0f * farZ * nearZ * b, a, a / aspectRatio);
	vf32 v1 = _mm_set_ps(0.0f, 0.0f, -1.0f, (farZ + nearZ) * b);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixPerspectiveLH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ / (farZ - nearZ);

	vf32 v0 = _mm_set_ps(0.0f, -nearZ * b, -a, a / aspectRatio);
	vf32 v1 = _mm_set_ps(0.0f, 0.0f, 1.0f, b);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixPerspectiveRH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ / (nearZ - farZ);

	vf32 v0 = _mm_set_ps(0.0f, nearZ * b, -a, a / aspectRatio);
	vf32 v1 = _mm_set_ps(0.0f, 0.0f, -1.0f, b);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 0, 3, 3));
	r.r[3] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixOrthographicOffCenterLH_DX(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (top - bottom);
	f32 oneOverFMinusN = 1.0f / (farZ - nearZ);

	vf32 v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, oneOverFMinusN, 0.0f);
	vf32 v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		-oneOverFMinusN * nearZ, 1.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixOrthographicOffCenterRH_DX(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (top - bottom);
	f32 oneOverNMinusF = 1.0f / (nearZ - farZ);

	vf32 v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, oneOverNMinusF, 0.0f);
	vf32 v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		oneOverNMinusF * nearZ, 1.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixOrthographicOffCenterLH_GL(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (top - bottom);
	f32 oneOverNMinusF = 1.0f / (nearZ - farZ);

	vf32 v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, -2.0f * oneOverNMinusF, 0.0f);
	vf32 v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		(nearZ + farZ) * oneOverNMinusF, 1.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixOrthographicOffCenterRH_GL(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (top - bottom);
	f32 oneOverFMinusN = 1.0f / (farZ - nearZ);

	vf32 v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, -2.0f * oneOverFMinusN, 0.0f);
	vf32 v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		-(nearZ + farZ) * oneOverFMinusN, 1.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixOrthographicOffCenterLH_VK(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (bottom - top);
	f32 oneOverFMinusN = 1.0f / (farZ - nearZ);

	vf32 v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, oneOverFMinusN, 0.0f);
	vf32 v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		-oneOverFMinusN * nearZ, 1.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

BV_INLINE mf32 BV_VCALL MatrixOrthographicOffCenterRH_VK(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (bottom - top);
	f32 oneOverNMinusF = 1.0f / (nearZ - farZ);

	vf32 v0 = VectorSet(oneOverRMinusL + oneOverRMinusL, oneOverTMinusB + oneOverTMinusB, oneOverNMinusF, 0.0f);
	vf32 v1 = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB,
		oneOverNMinusF * nearZ, 1.0f);

	mf32 r;
	r.r[0] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 3, 0));
	r.r[1] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 3, 1, 3));
	r.r[2] = _mm_shuffle_ps(v0, v0, _MM_SHUFFLE(3, 2, 3, 3));
	r.r[3] = v1;

	return r;
}

BV_INLINE mf32 BV_VCALL operator + (cmf32 m)
{
	return m;
}

BV_INLINE mf32 BV_VCALL operator - (cmf32 m)
{
	mf32 r;
	r.r[0] = VectorNegate(m.r[0]);
	r.r[1] = VectorNegate(m.r[1]);
	r.r[2] = VectorNegate(m.r[2]);
	r.r[3] = VectorNegate(m.r[3]);

	return r;
}

BV_INLINE mf32 BV_VCALL operator + (cmf32 m1, cmf32 m2)
{
	return MatrixAdd(m1, m2);
}

BV_INLINE mf32& BV_VCALL operator += (mf32& m1, cmf32 m2)
{
	m1 = MatrixAdd(m1, m2);
	return m1;
}

BV_INLINE mf32 BV_VCALL operator - (cmf32 m1, cmf32 m2)
{
	return MatrixSub(m1, m2);
}

BV_INLINE mf32& BV_VCALL operator -= (mf32& m1, cmf32 m2)
{
	m1 = MatrixSub(m1, m2);
	return m1;
}

BV_INLINE mf32 BV_VCALL operator * (cmf32 m1, cmf32 m2)
{
	return MatrixMul(m1, m2);
}

BV_INLINE mf32& BV_VCALL operator *= (mf32& m1, cmf32 m2)
{
	m1 = MatrixMul(m1, m2);
	return m1;
}

BV_INLINE mf32 BV_VCALL operator+(cmf32 m, f32 s)
{
	return MatrixAdd(m, s);
}

BV_INLINE mf32& BV_VCALL operator+=(mf32& m, f32 s)
{
	m = MatrixAdd(m, s);
	return m;
}

BV_INLINE mf32 BV_VCALL operator-(cmf32 m, f32 s)
{
	return MatrixSub(m, s);
}

BV_INLINE mf32& BV_VCALL operator-=(mf32& m, f32 s)
{
	m = MatrixSub(m, s);
	return m;
}

BV_INLINE mf32 BV_VCALL operator * (cmf32 m, f32 s)
{
	vf32 v = _mm_set1_ps(s);
	mf32 r;
	r.r[0] = _mm_mul_ps(m.r[0], v);
	r.r[1] = _mm_mul_ps(m.r[1], v);
	r.r[2] = _mm_mul_ps(m.r[2], v);
	r.r[3] = _mm_mul_ps(m.r[3], v);

	return r;
}

BV_INLINE mf32& BV_VCALL operator *= (mf32& m, f32 s)
{
	vf32 v = _mm_set1_ps(s);
	m.r[0] = _mm_mul_ps(m.r[0], v);
	m.r[1] = _mm_mul_ps(m.r[1], v);
	m.r[2] = _mm_mul_ps(m.r[2], v);
	m.r[3] = _mm_mul_ps(m.r[3], v);

	return m;
}

BV_INLINE mf32 BV_VCALL operator / (cmf32 m, f32 s)
{
	return MatrixDiv(m, s);
}

BV_INLINE mf32& BV_VCALL operator /= (mf32& m, f32 s)
{
	m = MatrixDiv(m, s);
	return m;
}

#else

inline mf32 MatrixSet(const vf32& v0, const vf32& v1, const vf32& v2, const vf32& v3)
{
	return mf32(v0, v1, v2, v3);
}

inline mf32 MatrixAdd(const mf32& m1, const mf32& m2)
{
	mf32 m;
	m.r[0] = VectorAdd(m1.r[0], m2.r[0]);
	m.r[1] = VectorAdd(m1.r[1], m2.r[1]);
	m.r[2] = VectorAdd(m1.r[2], m2.r[2]);
	m.r[3] = VectorAdd(m1.r[3], m2.r[3]);

	return m;
}

inline mf32 MatrixAdd(const mf32& m, f32 s)
{
	mf32 r;
	r.r[0] = VectorAdd(m.r[0], s);
	r.r[1] = VectorAdd(m.r[1], s);
	r.r[2] = VectorAdd(m.r[2], s);
	r.r[3] = VectorAdd(m.r[3], s);

	return r;
}

inline mf32 MatrixSub(const mf32& m1, const mf32& m2)
{
	mf32 m;
	m.r[0] = VectorSub(m1.r[0], m2.r[0]);
	m.r[1] = VectorSub(m1.r[1], m2.r[1]);
	m.r[2] = VectorSub(m1.r[2], m2.r[2]);
	m.r[3] = VectorSub(m1.r[3], m2.r[3]);

	return m;
}

inline mf32 MatrixSub(const mf32& m, f32 s)
{
	mf32 r;
	r.r[0] = VectorSub(m.r[0], s);
	r.r[1] = VectorSub(m.r[1], s);
	r.r[2] = VectorSub(m.r[2], s);
	r.r[3] = VectorSub(m.r[3], s);

	return r;
}

inline mf32 MatrixMul(const mf32& m1, const mf32& m2)
{
	mf32 m;
	m.r[0] = VectorSet(
		m1.r[0].x * m2.r[0].x + m1.r[0].y * m2.r[1].x + m1.r[0].z * m2.r[2].x + m1.r[0].w * m2.r[3].x,
		m1.r[0].x * m2.r[0].y + m1.r[0].y * m2.r[1].y + m1.r[0].z * m2.r[2].y + m1.r[0].w * m2.r[3].y,
		m1.r[0].x * m2.r[0].z + m1.r[0].y * m2.r[1].z + m1.r[0].z * m2.r[2].z + m1.r[0].w * m2.r[3].z,
		m1.r[0].x * m2.r[0].w + m1.r[0].y * m2.r[1].w + m1.r[0].z * m2.r[2].w + m1.r[0].w * m2.r[3].w
	);

	m.r[1] = VectorSet(
		m1.r[1].x * m2.r[0].x + m1.r[1].y * m2.r[1].x + m1.r[1].z * m2.r[2].x + m1.r[1].w * m2.r[3].x,
		m1.r[1].x * m2.r[0].y + m1.r[1].y * m2.r[1].y + m1.r[1].z * m2.r[2].y + m1.r[1].w * m2.r[3].y,
		m1.r[1].x * m2.r[0].z + m1.r[1].y * m2.r[1].z + m1.r[1].z * m2.r[2].z + m1.r[1].w * m2.r[3].z,
		m1.r[1].x * m2.r[0].w + m1.r[1].y * m2.r[1].w + m1.r[1].z * m2.r[2].w + m1.r[1].w * m2.r[3].w
	);

	m.r[2] = VectorSet(
		m1.r[2].x * m2.r[0].x + m1.r[2].y * m2.r[1].x + m1.r[2].z * m2.r[2].x + m1.r[2].w * m2.r[3].x,
		m1.r[2].x * m2.r[0].y + m1.r[2].y * m2.r[1].y + m1.r[2].z * m2.r[2].y + m1.r[2].w * m2.r[3].y,
		m1.r[2].x * m2.r[0].z + m1.r[2].y * m2.r[1].z + m1.r[2].z * m2.r[2].z + m1.r[2].w * m2.r[3].z,
		m1.r[2].x * m2.r[0].w + m1.r[2].y * m2.r[1].w + m1.r[2].z * m2.r[2].w + m1.r[2].w * m2.r[3].w
	);

	m.r[1] = VectorSet(
		m1.r[3].x * m2.r[0].x + m1.r[3].y * m2.r[1].x + m1.r[3].z * m2.r[2].x + m1.r[3].w * m2.r[3].x,
		m1.r[3].x * m2.r[0].y + m1.r[3].y * m2.r[1].y + m1.r[3].z * m2.r[2].y + m1.r[3].w * m2.r[3].y,
		m1.r[3].x * m2.r[0].z + m1.r[3].y * m2.r[1].z + m1.r[3].z * m2.r[2].z + m1.r[3].w * m2.r[3].z,
		m1.r[3].x * m2.r[0].w + m1.r[3].y * m2.r[1].w + m1.r[3].z * m2.r[2].w + m1.r[3].w * m2.r[3].w
	);

	return m;
}

inline mf32 MatrixMul(const mf32& m, f32 s)
{
	mf32 r;
	r.r[0] = VectorSet(
		m.r[0].x * s + m.r[0].y * s + m.r[0].z * s + m.r[0].w * s,
		m.r[0].x * s + m.r[0].y * s + m.r[0].z * s + m.r[0].w * s,
		m.r[0].x * s + m.r[0].y * s + m.r[0].z * s + m.r[0].w * s,
		m.r[0].x * s + m.r[0].y * s + m.r[0].z * s + m.r[0].w * s
	);

	r.r[1] = VectorSet(
		m.r[1].x * s + m.r[1].y * s + m.r[1].z * s + m.r[1].w * s,
		m.r[1].x * s + m.r[1].y * s + m.r[1].z * s + m.r[1].w * s,
		m.r[1].x * s + m.r[1].y * s + m.r[1].z * s + m.r[1].w * s,
		m.r[1].x * s + m.r[1].y * s + m.r[1].z * s + m.r[1].w * s
	);

	r.r[2] = VectorSet(
		m.r[2].x * s + m.r[2].y * s + m.r[2].z * s + m.r[2].w * s,
		m.r[2].x * s + m.r[2].y * s + m.r[2].z * s + m.r[2].w * s,
		m.r[2].x * s + m.r[2].y * s + m.r[2].z * s + m.r[2].w * s,
		m.r[2].x * s + m.r[2].y * s + m.r[2].z * s + m.r[2].w * s
	);

	r.r[1] = VectorSet(
		m.r[3].x * s + m.r[3].y * s + m.r[3].z * s + m.r[3].w * s,
		m.r[3].x * s + m.r[3].y * s + m.r[3].z * s + m.r[3].w * s,
		m.r[3].x * s + m.r[3].y * s + m.r[3].z * s + m.r[3].w * s,
		m.r[3].x * s + m.r[3].y * s + m.r[3].z * s + m.r[3].w * s
	);

	return r;
}

inline mf32 MatrixDiv(const mf32& m, f32 s)
{
	mf32 r;
	r.r[0] = VectorDiv(m.r[0], s);
	r.r[1] = VectorDiv(m.r[1], s);
	r.r[2] = VectorDiv(m.r[2], s);
	r.r[3] = VectorDiv(m.r[3], s);

	return r;
}

inline mf32 MatrixIdentity()
{
	mf32 m;
	m.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	m.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	m.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return m;
}

inline mf32 MatrixTranspose(const mf32& m)
{
	mf32 mT;
	mT.r[0] = VectorSet(m.r[0].x, m.r[1].x, m.r[2].x, m.r[3].x);
	mT.r[1] = VectorSet(m.r[0].y, m.r[1].y, m.r[2].y, m.r[3].y);
	mT.r[2] = VectorSet(m.r[0].z, m.r[1].z, m.r[2].z, m.r[3].z);
	mT.r[3] = VectorSet(m.r[0].w, m.r[1].w, m.r[2].w, m.r[3].w);

	return mT;
}

inline vf32 MatrixDeterminantV(const mf32& m)
{
	f32 a1 = m.r[2].z * m.r[3].w - m.r[2].w * m.r[3].z;
	f32 a2 = m.r[2].y * m.r[3].w - m.r[2].w * m.r[3].y;
	f32 a3 = m.r[2].y * m.r[3].z - m.r[2].z * m.r[3].y;
	f32 a4 = m.r[2].x * m.r[3].w - m.r[2].w * m.r[3].x;
	f32 a5 = m.r[2].x * m.r[3].z - m.r[2].z * m.r[3].x;
	f32 a6 = m.r[2].x * m.r[3].y - m.r[2].y * m.r[3].x;

	f32 det =
		m.r[0].x * (m.r[1].y * (a1)-m.r[1].z * (a2)+m.r[1].w * (a3))
		- m.r[0].y * (m.r[1].x * (a1)-m.r[1].z * (a4)+m.r[1].w * (a5))
		+ m.r[0].z * (m.r[1].x * (a2)-m.r[1].y * (a4)+m.r[1].w * (a6))
		- m.r[0].w * (m.r[1].x * (a3)-m.r[1].y * (a5)+m.r[1].z * (a6));

	return VectorSet(det, det, det, det);
}

inline f32 MatrixDeterminant(const mf32& m)
{
	return MatrixDeterminantV(m).x;
}

inline mf32 MatrixInverse(const mf32& m)
{
	mf32 mT = MatrixTranspose(m);

	// First 12 coefficients
	f32 tmp[12];
	tmp[0] = mT.r[2].z * mT.r[3].w;
	tmp[1] = mT.r[2].w * mT.r[3].z;
	tmp[2] = mT.r[2].y * mT.r[3].w;
	tmp[3] = mT.r[2].w * mT.r[3].y;
	tmp[4] = mT.r[2].y * mT.r[3].z;
	tmp[5] = mT.r[2].z * mT.r[3].y;
	tmp[6] = mT.r[2].x * mT.r[3].w;
	tmp[7] = mT.r[2].w * mT.r[3].x;
	tmp[8] = mT.r[2].x * mT.r[3].z;
	tmp[9] = mT.r[2].z * mT.r[3].x;
	tmp[10] = mT.r[2].x * mT.r[3].y;
	tmp[11] = mT.r[2].y * mT.r[3].x;

	mf32 r;
	r.r[0].x = tmp[0] * mT.r[1].y + tmp[3] * mT.r[1].z + tmp[4] * mT.r[1].w;
	r.r[0].x -= tmp[1] * mT.r[1].y + tmp[2] * mT.r[1].z + tmp[5] * mT.r[1].w;
	r.r[0].y = tmp[1] * mT.r[1].x + tmp[6] * mT.r[1].z + tmp[9] * mT.r[1].w;
	r.r[0].y -= tmp[0] * mT.r[1].x + tmp[7] * mT.r[1].z + tmp[8] * mT.r[1].w;
	r.r[0].z = tmp[2] * mT.r[1].x + tmp[7] * mT.r[1].y + tmp[10] * mT.r[1].w;
	r.r[0].z -= tmp[3] * mT.r[1].x + tmp[6] * mT.r[1].y + tmp[11] * mT.r[1].w;
	r.r[0].w = tmp[5] * mT.r[1].x + tmp[8] * mT.r[1].y + tmp[11] * mT.r[1].z;
	r.r[0].w -= tmp[4] * mT.r[1].x + tmp[9] * mT.r[1].y + tmp[10] * mT.r[1].z;
	r.r[1].x = tmp[1] * mT.r[0].y + tmp[2] * mT.r[0].z + tmp[5] * mT.r[0].w;
	r.r[1].x -= tmp[0] * mT.r[0].y + tmp[3] * mT.r[0].z + tmp[4] * mT.r[0].w;
	r.r[1].y = tmp[0] * mT.r[0].x + tmp[7] * mT.r[0].z + tmp[8] * mT.r[0].w;
	r.r[1].y -= tmp[1] * mT.r[0].x + tmp[6] * mT.r[0].z + tmp[9] * mT.r[0].w;
	r.r[1].z = tmp[3] * mT.r[0].x + tmp[6] * mT.r[0].y + tmp[11] * mT.r[0].w;
	r.r[1].z -= tmp[2] * mT.r[0].x + tmp[7] * mT.r[0].y + tmp[10] * mT.r[0].w;
	r.r[1].w = tmp[4] * mT.r[0].x + tmp[9] * mT.r[0].y + tmp[10] * mT.r[0].z;
	r.r[1].w -= tmp[5] * mT.r[0].x + tmp[8] * mT.r[0].y + tmp[11] * mT.r[0].z;

	// Second 12 coefficients
	tmp[0] = mT.r[0].z * mT.r[1].w;
	tmp[1] = mT.r[0].w * mT.r[1].z;
	tmp[2] = mT.r[0].y * mT.r[1].w;
	tmp[3] = mT.r[0].w * mT.r[1].y;
	tmp[4] = mT.r[0].y * mT.r[1].z;
	tmp[5] = mT.r[0].z * mT.r[1].y;
	tmp[6] = mT.r[0].x * mT.r[1].w;
	tmp[7] = mT.r[0].w * mT.r[1].x;
	tmp[8] = mT.r[0].x * mT.r[1].z;
	tmp[9] = mT.r[0].z * mT.r[1].x;
	tmp[10] = mT.r[0].x * mT.r[1].y;
	tmp[11] = mT.r[0].y * mT.r[1].x;

	r.r[2].x = tmp[0] * mT.r[3].y + tmp[3] * mT.r[3].z + tmp[4] * mT.r[3].w;
	r.r[2].x -= tmp[1] * mT.r[3].y + tmp[2] * mT.r[3].z + tmp[5] * mT.r[3].w;
	r.r[2].y = tmp[1] * mT.r[3].x + tmp[6] * mT.r[3].z + tmp[9] * mT.r[3].w;
	r.r[2].y -= tmp[0] * mT.r[3].x + tmp[7] * mT.r[3].z + tmp[8] * mT.r[3].w;
	r.r[2].z = tmp[2] * mT.r[3].x + tmp[7] * mT.r[3].y + tmp[10] * mT.r[3].w;
	r.r[2].z -= tmp[3] * mT.r[3].x + tmp[6] * mT.r[3].y + tmp[11] * mT.r[3].w;
	r.r[2].w = tmp[5] * mT.r[3].x + tmp[8] * mT.r[3].y + tmp[11] * mT.r[3].z;
	r.r[2].w -= tmp[4] * mT.r[3].x + tmp[9] * mT.r[3].y + tmp[10] * mT.r[3].z;
	r.r[3].x = tmp[2] * mT.r[2].z + tmp[5] * mT.r[2].w + tmp[1] * mT.r[2].y;
	r.r[3].x -= tmp[4] * mT.r[2].w + tmp[0] * mT.r[2].y + tmp[3] * mT.r[2].z;
	r.r[3].y = tmp[8] * mT.r[2].w + tmp[0] * mT.r[2].x + tmp[7] * mT.r[2].z;
	r.r[3].y -= tmp[6] * mT.r[2].z + tmp[9] * mT.r[2].w + tmp[1] * mT.r[2].x;
	r.r[3].z = tmp[6] * mT.r[2].y + tmp[11] * mT.r[2].w + tmp[3] * mT.r[2].x;
	r.r[3].z -= tmp[10] * mT.r[2].w + tmp[2] * mT.r[2].x + tmp[7] * mT.r[2].y;
	r.r[3].w = tmp[10] * mT.r[2].z + tmp[4] * mT.r[2].x + tmp[9] * mT.r[2].y;
	r.r[3].w -= tmp[8] * mT.r[2].y + tmp[11] * mT.r[2].z + tmp[5] * mT.r[2].x;

	f32 det = 1.0f / (mT.r[0].x * r.r[0].x + mT.r[0].y * r.r[0].y + mT.r[0].z * r.r[0].z + mT.r[0].w * r.r[0].w);

	r.r[0].x *= det;
	r.r[0].y *= det;
	r.r[0].z *= det;
	r.r[0].w *= det;
	r.r[1].x *= det;
	r.r[1].y *= det;
	r.r[1].z *= det;
	r.r[1].w *= det;
	r.r[2].x *= det;
	r.r[2].y *= det;
	r.r[2].z *= det;
	r.r[2].w *= det;
	r.r[3].x *= det;
	r.r[3].y *= det;
	r.r[3].z *= det;
	r.r[3].w *= det;
}

inline mf32 operator + (const mf32& m)
{
	return m;
}

inline mf32 operator - (const mf32& m)
{
	mf32 r;
	r.r[0] = VectorNegate(m.r[0]);
	r.r[1] = VectorNegate(m.r[1]);
	r.r[2] = VectorNegate(m.r[2]);
	r.r[3] = VectorNegate(m.r[3]);

	return r;
}

inline mf32 operator + (const mf32& m1, const mf32& m2)
{
	return MatrixAdd(m1, m2);
}

inline mf32& operator += (mf32& m1, const mf32& m2)
{
	m1 = MatrixAdd(m1, m2);
	return m1;
}

inline mf32 operator - (const mf32& m1, const mf32& m2)
{
	return MatrixSub(m1, m2);
}

inline mf32& operator -= (mf32& m1, const mf32& m2)
{
	m1 = MatrixSub(m1, m2);
	return m1;
}

inline mf32 operator * (const mf32& m1, const mf32& m2)
{
	return MatrixMul(m1, m2);
}

inline mf32& operator *= (mf32& m1, const mf32& m2)
{
	m1 = MatrixMul(m1, m2);
	return m1;
}

inline mf32 operator + (const mf32& m, f32 s)
{
	vf32 v = VectorReplicate(s);

	mf32 r;
	r.r[0] = VectorAdd(m.r[0], v);
	r.r[1] = VectorAdd(m.r[1], v);
	r.r[2] = VectorAdd(m.r[2], v);
	r.r[3] = VectorAdd(m.r[3], v);

	return r;
}

inline mf32& operator += (mf32& m, f32 s)
{
	m.r[0] = VectorAdd(m.r[0], s);
	m.r[1] = VectorAdd(m.r[1], s);
	m.r[2] = VectorAdd(m.r[2], s);
	m.r[3] = VectorAdd(m.r[3], s);

	return m;
}

inline mf32 operator - (const mf32& m, f32 s)
{
	vf32 v = VectorReplicate(s);

	mf32 r;
	r.r[0] = VectorSub(m.r[0], v);
	r.r[1] = VectorSub(m.r[1], v);
	r.r[2] = VectorSub(m.r[2], v);
	r.r[3] = VectorSub(m.r[3], v);

	return r;
}

inline mf32& operator -= (mf32& m, f32 s)
{
	m.r[0] = VectorSub(m.r[0], s);
	m.r[1] = VectorSub(m.r[1], s);
	m.r[2] = VectorSub(m.r[2], s);
	m.r[3] = VectorSub(m.r[3], s);

	return m;
}

inline mf32 operator * (const mf32& m, f32 s)
{
	vf32 v = VectorReplicate(s);

	mf32 r;
	r.r[0] = VectorMul(m.r[0], v);
	r.r[1] = VectorMul(m.r[1], v);
	r.r[2] = VectorMul(m.r[2], v);
	r.r[3] = VectorMul(m.r[3], v);

	return r;
}

inline mf32& operator *= (mf32& m, f32 s)
{
	m.r[0] = VectorMul(m.r[0], s);
	m.r[1] = VectorMul(m.r[1], s);
	m.r[2] = VectorMul(m.r[2], s);
	m.r[3] = VectorMul(m.r[3], s);

	return m;
}

inline mf32 operator / (const mf32& m, f32 s)
{
	return MatrixDiv(m, s);
}

inline mf32& operator /= (mf32& m, f32 s)
{
	m = MatrixDiv(m, s);
	return m;
}

inline mf32 MatrixScaling(f32 sX, f32 sY, f32 sZ)
{
	mf32 r;
	r.r[0] = VectorSet(sX, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, sY, 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, sZ, 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline mf32 MatrixTranslation(f32 tX, f32 tY, f32 tZ)
{
	mf32 r;
	r.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = VectorSet(tX, tY, tZ, 1.0f);

	return r;
}

inline mf32 MatrixRotationX(f32 angle)
{
	f32 cosSin[] = { cosf(angle), sinf(angle) };

	mf32 r;
	r.r[0] = VectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	r.r[1] = VectorSet(0.0f, cosSin[0], cosSin[1], 0.0f);
	r.r[2] = VectorSet(0.0f, -cosSin[1], cosSin[0], 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline mf32 MatrixRotationY(f32 angle)
{
	f32 cosSin[] = { cosf(angle), sinf(angle) };

	mf32 r;
	r.r[0] = VectorSet(cosSin[0], 0.0f, -cosSin[1], 0.0f);
	r.r[1] = VectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	r.r[2] = VectorSet(cosSin[1], 0.0f, cosSin[0], 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline mf32 MatrixRotationZ(f32 angle)
{
	f32 cosSin[] = { cosf(angle), sinf(angle) };

	mf32 r;
	r.r[0] = VectorSet(cosSin[0], cosSin[1], 0.0f, 0.0f);
	r.r[1] = VectorSet(-cosSin[1], cosSin[0], 0.0f, 0.0f);
	r.r[2] = VectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	r.r[3] = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	return r;
}

inline mf32 MatrixRotationAxis(const vf32& axis, f32 angle)
{
	f32 cosAngle = cosf(angle);
	f32 sinAngle = sinf(angle);
	f32 oneMinusCos = 1.0f - cosAngle;

	mf32 r;
	r.r[0].x = cosAngle + axis.x * axis.x * oneMinusCos;
	r.r[0].y = axis.y * axis.x * oneMinusCos + axis.z * sinAngle;
	r.r[0].z = axis.z * axis.x * oneMinusCos - axis.y * sinAngle;
	r.r[0].w = 0.0f;

	r.r[1].x = axis.x * axis.y * oneMinusCos - axis.z * sinAngle;
	r.r[1].y = cosAngle + axis.y * axis.y * oneMinusCos;
	r.r[1].z = axis.z * axis.y * oneMinusCos + axis.x * sinAngle;
	r.r[1].w = 0.0f;

	r.r[3].x = axis.x * axis.z * oneMinusCos + axis.y * sinAngle;
	r.r[3].y = axis.y * axis.z * oneMinusCos - axis.x * sinAngle;
	r.r[3].z = cosAngle + axis.z * axis.z * oneMinusCos;
	r.r[3].w = 0.0f;

	r.r[3].x = r.r[3].y = r.r[3].z = 0.0f;
	r.r[3].w = 1.0f;

	return r;
}

inline mf32 MatrixLookAt(const vf32& eyePos, const vf32& dirVec, const vf32& upVec)
{
	vf32 z = VectorNormalize(dirVec);

	vf32 x = VectorCross(upVec, z);
	x = VectorNormalize(x);

	vf32 y = VectorCross(z, x);

	vf32 w = VectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	mf32 r(x, y, z, w);

	r = MatrixTranspose(r);
	r.r[3] = VectorSet(-VectorDot(x, eyePos), -VectorDot(y, eyePos), -VectorDot(z, eyePos), 1.0f);

	return r;
}

inline mf32 MatrixLookAtLH(const vf32& eyePos, const vf32& lookPos, const vf32& upVec)
{
	vf32 dir = VectorSub(lookPos, eyePos);
	return MatrixLookAt(eyePos, dir, upVec);
}

inline mf32 MatrixLookAtRH(const vf32& eyePos, const vf32& lookPos, const vf32& upVec)
{
	vf32 dir = VectorSub(eyePos, lookPos);
	return MatrixLookAt(eyePos, dir, upVec);
}

inline mf32 MatrixPerspectiveLH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ / (farZ - nearZ);

	f32 vals[] = { a / aspectRatio, a, b, 1.0f, -nearZ * b };

	mf32 r;
	r.r[0].x = vals[0];
	r.r[1].y = vals[1];
	r.r[2].z = vals[2];
	r.r[2].w = vals[3];
	r.r[3].z = vals[4];

	return r;
}

inline mf32 MatrixPerspectiveRH_DX(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ / (farZ - nearZ);

	f32 vals[] = { a / aspectRatio, a, b, -1.0f, nearZ * b };

	mf32 r;
	r.r[0].x = vals[0];
	r.r[1].y = vals[1];
	r.r[2].z = vals[2];
	r.r[2].w = vals[3];
	r.r[3].z = vals[4];

	return r;
}

inline mf32 MatrixPerspectiveLH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ - nearZ;

	f32 vals[] = { a / aspectRatio, a, -(farZ + nearZ) / b, 1.0f, (2.0f * farZ * nearZ) / b };

	mf32 r;
	r.r[0].x = vals[0];
	r.r[1].y = vals[1];
	r.r[2].z = vals[2];
	r.r[2].w = vals[3];
	r.r[3].z = vals[4];

	return r;
}

inline mf32 MatrixPerspectiveRH_GL(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ - nearZ;

	f32 vals[] = { a / aspectRatio, a, -(farZ + nearZ) / b, -1.0f, (-2.0f * farZ * nearZ) / b };

	mf32 r;
	r.r[0].x = vals[0];
	r.r[1].y = vals[1];
	r.r[2].z = vals[2];
	r.r[2].w = vals[3];
	r.r[3].z = vals[4];

	return r;
}

inline mf32 MatrixPerspectiveLH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ / (farZ - nearZ);

	f32 vals[] = { a / aspectRatio, -a, b, 1.0f, -nearZ * b };

	mf32 r;
	r.r[0].x = vals[0];
	r.r[1].y = vals[1];
	r.r[2].z = vals[2];
	r.r[2].w = vals[3];
	r.r[3].z = vals[4];

	return r;
}

inline mf32 MatrixPerspectiveRH_VK(f32 nearZ, f32 farZ, f32 aspectRatio, f32 fovY)
{
	f32 a = 1.0f / tanf(fovY * 0.5f);
	f32 b = farZ / (farZ - nearZ);

	f32 vals[] = { a / aspectRatio, -a, b, -1.0f, nearZ * b };

	mf32 r;
	r.r[0].x = vals[0];
	r.r[1].y = vals[1];
	r.r[2].z = vals[2];
	r.r[2].w = vals[3];
	r.r[3].z = vals[4];

	return r;
}

inline mf32 MatrixOrthographicOffCenterLH_DX(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (top - bottom);
	f32 oneOverFMinusN = 1.0f / (farZ - nearZ);

	mf32 r;
	r.r[0].x = oneOverRMinusL + oneOverRMinusL;
	r.r[1].y = oneOverTMinusB + oneOverTMinusB;
	r.r[2].z = oneOverFMinusN;
	r.r[3] = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB, -oneOverFMinusN * nearZ, 1.0f);

	return r;
}

inline mf32 MatrixOrthographicOffCenterRH_DX(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (top - bottom);
	f32 oneOverNMinusF = 1.0f / (nearZ - farZ);

	mf32 r;
	r.r[0].x = oneOverRMinusL + oneOverRMinusL;
	r.r[1].y = oneOverTMinusB + oneOverTMinusB;
	r.r[2].z = oneOverNMinusF;
	r.r[3] = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB, oneOverNMinusF * nearZ, 1.0f);

	return r;
}

inline mf32 MatrixOrthographicOffCenterLH_GL(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (top - bottom);
	f32 oneOverNMinusF = 1.0f / (nearZ - farZ);

	mf32 r;
	r.r[0].x = oneOverRMinusL + oneOverRMinusL;
	r.r[1].y = oneOverTMinusB + oneOverTMinusB;
	r.r[2].z = -2.0f * oneOverNMinusF;
	r.r[3] = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB, (nearZ + farZ) * oneOverNMinusF, 1.0f);

	return r;
}

inline mf32 MatrixOrthographicOffCenterRH_GL(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (top - bottom);
	f32 oneOverFMinusN = 1.0f / (farZ - nearZ);

	mf32 r;
	r.r[0].x = oneOverRMinusL + oneOverRMinusL;
	r.r[1].y = oneOverTMinusB + oneOverTMinusB;
	r.r[2].z = -2.0f * oneOverFMinusN;
	r.r[3] = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB, -(nearZ + farZ) * oneOverFMinusN, 1.0f);

	return r;
}

inline mf32 MatrixOrthographicOffCenterLH_VK(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (bottom - top);
	f32 oneOverFMinusN = 1.0f / (farZ - nearZ);

	mf32 r;
	r.r[0].x = oneOverRMinusL + oneOverRMinusL;
	r.r[1].y = oneOverTMinusB + oneOverTMinusB;
	r.r[2].z = oneOverFMinusN;
	r.r[3] = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB, -oneOverFMinusN * nearZ, 1.0f);

	return r;
}

inline mf32 MatrixOrthographicOffCenterRH_VK(f32 right, f32 left, f32 top, f32 bottom, f32
	nearZ, f32 farZ)
{
	f32 oneOverRMinusL = 1.0f / (right - left);
	f32 oneOverTMinusB = 1.0f / (bottom - top);
	f32 oneOverNMinusF = 1.0f / (nearZ - farZ);

	mf32 r;
	r.r[0].x = oneOverRMinusL + oneOverRMinusL;
	r.r[1].y = oneOverTMinusB + oneOverTMinusB;
	r.r[2].z = oneOverNMinusF;
	r.r[3] = VectorSet(-(left + right) * oneOverRMinusL, -(bottom + top) * oneOverTMinusB, oneOverNMinusF * nearZ, 1.0f);

	return r;
}

#endif
#pragma once


#include "BvFastVec_FPU.inl"


#if (BV_MATH_INSTRUCTION == BV_MATH_INSTRUCTION_FPU)


// ==================================
// Quaternion operations
// ==================================

// ======================
// Declarations
// ======================

// ======================
// Specialized Operations
// ======================

qf32 QuaternionIdentity();

qf32 QuaternionMul(crqf32 q1, crqf32 q2);
qf32 QuaternionMulKeenan(crqf32 q1, crqf32 q2);

qf32 QuaternionConjugate(crqf32 q);
qf32 QuaternionInverse(crqf32 q);

qf32 QuaternionNormalize(crqf32 q);

f32 QuaternionDot(crqf32 q1, crqf32 q2);
qf32 QuaternionDotV(crqf32 q1, crqf32 q2);

f32 QuaternionLengthSqr(crqf32 q);
qf32 QuaternionLengthSqrV(crqf32 q);

f32 QuaternionLength(crqf32 q);
qf32 QuaternionLengthV(crqf32 q);

qf32 QuaternionRotationAxis(const vf32 & v, f32 angle);
vf32 QuaternionQVQC(crqf32 q, const vf32 & v);
vf32 QuaternionQCVQ(crqf32 q, const vf32 & v);

vf32 QuaternionQVQCKeenan(crqf32 q, const vf32 & v);
vf32 QuaternionQCVQKeenan(crqf32 q, const vf32 & v);

mf32 QuaternionToMatrix(crqf32 q);
qf32 QuaternionFromMatrix(const mf32 & m);

qf32 QuaternionSlerp(crqf32 q1, crqf32 q2, f32 t, f32 epsilon = kEpsilon);

f32 QuaternionAngle(crqf32 q);

// =================
// Definitions
// =================

// ======================
// Specialized Operations
// ======================

inline qf32 QuaternionIdentity()
{
	return qf32(0.0f, 0.0f, 0.0f, 1.0f);
}

inline qf32 QuaternionMul(crqf32 q1, crqf32 q2)
{
	// Quaternion Multiplication
	// [q1s * q2v + q2s * q1v + q1 x q2] [q1s * q2s - q1.q2]
	// Expanding the equation
	// x => q1.w * q2.x + q2.w * q1.x + q1.y * q2.z - q1.z * q2.y
	// y => q1.w * q2.y + q2.w * q1.y + q1.z * q2.x - q1.x * q2.z
	// z => q1.w * q2.z + q2.w * q1.z + q1.x * q2.y - q1.y * q2.x
	// w => q1.w * q2.w - q2.x * q1.x - q1.y * q2.y - q1.z * q2.z

	return VectorSet(
			q1.w * q2.x + q2.w * q1.x + q1.y * q2.z - q1.z * q2.y,
			q1.w * q2.y + q2.w * q1.y + q1.z * q2.x - q1.x * q2.z,
			q1.w * q2.z + q2.w * q1.z + q1.x * q2.y - q1.y * q2.x,
			q1.w * q2.w - q2.x * q1.x - q1.y * q2.y - q1.z * q2.z
		);
}

inline qf32 QuaternionMulKeenan(crqf32 q1, crqf32 q2)
{
	// Quaternion Multiplication
	// [q1s * q2v + q2s * q1v + q2 x q1] [q1s * q2s - q1.q2]
	// Expanding the equation
	// x => q1.w * q2.x + q2.w * q1.x + q2.y * q1.z - q2.z * q1.y
	// y => q1.w * q2.y + q2.w * q1.y + q2.z * q1.x - q2.x * q1.z
	// z => q1.w * q2.z + q2.w * q1.z + q2.x * q1.y - q2.y * q1.x
	// w => q1.w * q2.w - q2.x * q1.x - q2.y * q1.y - q2.z * q1.z

	return VectorSet(
			q1.w * q2.x + q2.w * q1.x + q2.y * q1.z - q2.z * q1.y,
			q1.w * q2.y + q2.w * q1.y + q2.z * q1.x - q2.x * q1.z,
			q1.w * q2.z + q2.w * q1.z + q2.x * q1.y - q2.y * q1.x,
			q1.w * q2.w - q2.x * q1.x - q2.y * q1.y - q2.z * q1.z
	);
}

inline qf32 QuaternionConjugate(crqf32 q)
{
	return VectorSet(-q.x, -q.y, -q.z, q.w);
}

inline qf32 QuaternionInverse(crqf32 q)
{
	qf32 conj = QuaternionConjugate(q);
	f32 oneOverLengthSqr = 1.0f / (QuaternionLengthSqr(q));

	return qf32(
		conj.x * oneOverLengthSqr,
		conj.y * oneOverLengthSqr,
		conj.z * oneOverLengthSqr,
		conj.w * oneOverLengthSqr
		);
}

inline qf32 QuaternionNormalize(crqf32 q)
{
	f32 oneOverLengthSqr = QuaternionLengthSqr(q);

	return qf32(
		q.x * oneOverLengthSqr,
		q.y * oneOverLengthSqr,
		q.z * oneOverLengthSqr,
		q.w * oneOverLengthSqr
	);
}

inline f32 QuaternionDot(crqf32 q1, crqf32 q2)
{
	// _mm_cvtss_f32 gets the value of the lowest f32, in this case, X
	// _mm_store_ss could also be used, but I think it might be slower
	return QuaternionDotV(q1, q2).x;
}

inline qf32 QuaternionDotV(crqf32 q1, crqf32 q2)
{
	return VectorReplicate(q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);
}

inline f32 QuaternionLengthSqr(crqf32 q)
{
	return QuaternionLengthSqrV(q).x;
}

inline qf32 QuaternionLengthSqrV(crqf32 q)
{
	return QuaternionDotV(q, q);
}

inline f32 QuaternionLength(crqf32 v)
{
	return QuaternionLengthV(v).x;
}

inline qf32 QuaternionLengthV(crqf32 v)
{
	f32 oneOverLengthSqr = 1.0f / QuaternionLengthSqr(v);
	return qf32(v.x * oneOverLengthSqr, v.y * oneOverLengthSqr, v.z * oneOverLengthSqr, v.w * oneOverLengthSqr);
}

inline qf32 QuaternionRotationAxis(const vf32 & v, f32 angle)
{
	f32 halfAngle = angle * 0.5f;
	f32 sinCos[] = { sinf(halfAngle), cosf(halfAngle) };

	return vf32(v.x * sinCos[0], v.y * sinCos[0], v.z * sinCos[0], sinCos[1]);
}

inline vf32 QuaternionQVQC(crqf32 q, const vf32 & v)
{
	f32 w2MinusQDotQ = q.w * q.w - VectorDot(q, q);
	f32 twoTimesQDotV = 2.0f * VectorDot(q, v);
	f32 wTimesTwo = 2.0f * q.w;
	vf32 qCrossV = VectorCross(q, v);

	return vf32(w2MinusQDotQ * v.x + twoTimesQDotV * q.x + wTimesTwo * qCrossV.x,
		w2MinusQDotQ * v.y + twoTimesQDotV * q.y + wTimesTwo * qCrossV.y,
		w2MinusQDotQ * v.z + twoTimesQDotV * q.z + wTimesTwo * qCrossV.z);
}

inline vf32 QuaternionQCVQ(crqf32 q, const vf32 & v)
{
	f32 w2MinusQDotQ = q.w * q.w - VectorDot(q, q);
	f32 twoTimesQDotV = 2.0f * VectorDot(q, v);
	f32 wTimesTwo = 2.0f * q.w;
	vf32 vCrossQ = VectorCross(v, q);

	return vf32(w2MinusQDotQ * v.x + twoTimesQDotV * q.x + wTimesTwo * vCrossQ.x,
		w2MinusQDotQ * v.y + twoTimesQDotV * q.y + wTimesTwo * vCrossQ.y,
		w2MinusQDotQ * v.z + twoTimesQDotV * q.z + wTimesTwo * vCrossQ.z);
}
		
inline vf32 QuaternionQVQCKeenan(crqf32 q, const vf32 & v)
{
	return QuaternionQCVQ(q, v);
}

inline vf32 QuaternionQCVQKeenan(crqf32 q, const vf32 & v)
{
	return QuaternionQVQC(q, v);
}

inline mf32 QuaternionToMatrix(crqf32 q)
{
	f32 xy = q.x * q.y;
	f32 xz = q.x * q.z;
	f32 xw = q.x * q.w;

	f32 yz = q.x * q.z;
	f32 yw = q.x * q.w;

	f32 zw = q.x * q.w;

	f32 xx = q.x * q.x;
	f32 yy = q.y * q.y;
	f32 zz = q.z * q.z;

	mf32 r;
	r.r[0].x = 1.0f - 2.0f * (yy + zz);
	r.r[0].y = 2.0f * (xy + zw);
	r.r[0].z = 2.0f * (xz - yw);
	r.r[0].w = 0.0f;

	r.r[1].x = 2.0f * (xy - zw);
	r.r[1].y = 1.0f - 2.0f * (xx + zz);
	r.r[1].z = 2.0f * (yz + xw);
	r.r[1].w = 0.0f;

	r.r[2].x = 2.0f * (xz + yw);
	r.r[2].y = 2.0f * (yz - xw);
	r.r[2].z = 1.0f - 2.0f * (xx + yy);
	r.r[2].w = 0.0f;

	r.r[3].x = 0.0f;
	r.r[3].y = 0.0f;
	r.r[3].z = 0.0f;
	r.r[3].w = 1.0f;

	return r;
}

inline qf32 QuaternionFromMatrix(const mf32 & m)
{
	f32 trace = m.r[0].x + m.r[1].y + m.r[2].z;

	qf32 q;
	if (trace > 0.0f)
	{
		f32 s = sqrtf(trace + 1.0f) * 2.0f; // S=4*qw 
		q.w = 0.25f * s;
		q.x = (m.r[2].y - m.r[1].z) / s;
		q.y = (m.r[0].z - m.r[2].x) / s;
		q.z = (m.r[1].x - m.r[0].y) / s;
	}
	else if ((m.r[0].x > m.r[1].y) && (m.r[0].x > m.r[2].z))
	{
		f32 s = sqrtf(1.0f + m.r[0].x - m.r[1].y - m.r[2].z) * 2.0f; // S=4*qx 
		q.w = (m.r[2].y - m.r[1].z) / s;
		q.x = 0.25f * s;
		q.y = (m.r[0].y + m.r[1].x) / s;
		q.z = (m.r[0].z + m.r[2].x) / s;
	}
	else if (m.r[1].y > m.r[2].z)
	{
		f32 s = sqrtf(1.0f + m.r[1].y - m.r[0].x - m.r[2].z) * 2.0f; // S=4*qy
		q.w = (m.r[0].z - m.r[2].x) / s;
		q.x = (m.r[0].y + m.r[1].x) / s;
		q.y = 0.25f * s;
		q.z = (m.r[1].z + m.r[2].y) / s;
	}
	else
	{
		f32 s = sqrtf(1.0f + m.r[2].z - m.r[0].x - m.r[1].y) * 2.0f; // S=4*qz
		q.w = (m.r[1].x - m.r[0].y) / s;
		q.x = (m.r[0].z + m.r[2].x) / s;
		q.y = (m.r[1].z + m.r[2].y) / s;
		q.z = 0.25f * s;
	}

	return QuaternionNormalize(q);
}

inline qf32 QuaternionSlerp(crqf32 q1, crqf32 q2, f32 t, f32 epsilon)
{
	f32 cosOmega = QuaternionDot(q1, q2);
	qf32 qt(q2);
	if (cosOmega < 0.0f)
	{
		cosOmega = -cosOmega;
		qt = QuaternionConjugate(qt);
	}

	f32 omega = acosf(cosOmega);

	f32 mult1 = 1.0f - t;
	f32 mult2 = t;
	if (omega > epsilon)
	{
		f32 oneOverSinOmega = 1.0f / sinf(omega);
		mult1 = sinf(mult1 * omega) * oneOverSinOmega;
		mult2 = sinf(mult2 * omega) * oneOverSinOmega;
	}

	qf32 r = q1 * mult1 + qt * mult2;

	return QuaternionNormalize(r);
}

inline f32 QuaternionAngle(crqf32 q)
{
	return 2.0f * acosf(q.w);
}


#endif
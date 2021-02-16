#pragma once


#include "BvFastVec_FPU.inl"


#if (BV_MATH_USE_TYPE == BV_MATH_TYPE_FPU)


// ==================================
// Quaternion operations
// ==================================

// ======================
// Declarations
// ======================

// ======================
// Specialized Operations
// ======================

BvFastQuat QuaternionIdentity();

BvFastQuat QuaternionMul(const BvFastQuat & q1, const BvFastQuat & q2);
BvFastQuat QuaternionMulKeenan(const BvFastQuat & q1, const BvFastQuat & q2);

BvFastQuat QuaternionConjugate(const BvFastQuat & q);
BvFastQuat QuaternionInverse(const BvFastQuat & q);

BvFastQuat QuaternionNormalize(const BvFastQuat & q);

float QuaternionDot(const BvFastQuat & q1, const BvFastQuat & q2);
BvFastQuat QuaternionDotV(const BvFastQuat & q1, const BvFastQuat & q2);

float QuaternionLengthSqr(const BvFastQuat & q);
BvFastQuat QuaternionLengthSqrV(const BvFastQuat & q);

float QuaternionLength(const BvFastQuat & q);
BvFastQuat QuaternionLengthV(const BvFastQuat & q);

BvFastQuat QuaternionRotationAxis(const BvFastVec & v, float angle);
BvFastVec QuaternionQVQC(const BvFastQuat & q, const BvFastVec & v);
BvFastVec QuaternionQCVQ(const BvFastQuat & q, const BvFastVec & v);

BvFastVec QuaternionQVQCKeenan(const BvFastQuat & q, const BvFastVec & v);
BvFastVec QuaternionQCVQKeenan(const BvFastQuat & q, const BvFastVec & v);

BvFastMat QuaternionToMatrix(const BvFastQuat & q);
BvFastQuat QuaternionFromMatrix(const BvFastMat & m);

BvFastQuat QuaternionSlerp(const BvFastQuat & q1, const BvFastQuat & q2, const float t, const float epsilon = BvEpsilon);

float QuaternionAngle(const BvFastQuat & q);

// =================
// Definitions
// =================

// ======================
// Specialized Operations
// ======================

inline BvFastQuat QuaternionIdentity()
{
	return BvFastQuat(0.0f, 0.0f, 0.0f, 1.0f);
}

inline BvFastQuat QuaternionMul(const BvFastQuat & q1, const BvFastQuat & q2)
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

inline BvFastQuat QuaternionMulKeenan(const BvFastQuat & q1, const BvFastQuat & q2)
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

inline BvFastQuat QuaternionConjugate(const BvFastQuat & q)
{
	return VectorSet(-q.x, -q.y, -q.z, q.w);
}

inline BvFastQuat QuaternionInverse(const BvFastQuat & q)
{
	BvFastQuat conj = QuaternionConjugate(q);
	float oneOverLengthSqr = 1.0f / (QuaternionLengthSqr(q));

	return BvFastQuat(
		conj.x * oneOverLengthSqr,
		conj.y * oneOverLengthSqr,
		conj.z * oneOverLengthSqr,
		conj.w * oneOverLengthSqr
		);
}

inline BvFastQuat QuaternionNormalize(const BvFastQuat & q)
{
	float oneOverLengthSqr = QuaternionLengthSqr(q);

	return BvFastQuat(
		q.x * oneOverLengthSqr,
		q.y * oneOverLengthSqr,
		q.z * oneOverLengthSqr,
		q.w * oneOverLengthSqr
	);
}

inline float QuaternionDot(const BvFastQuat & q1, const BvFastQuat & q2)
{
	// _mm_cvtss_f32 gets the value of the lowest float, in this case, X
	// _mm_store_ss could also be used, but I think it might be slower
	return QuaternionDotV(q1, q2).x;
}

inline BvFastQuat QuaternionDotV(const BvFastQuat & q1, const BvFastQuat & q2)
{
	return VectorReplicate(q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);
}

inline float QuaternionLengthSqr(const BvFastQuat & q)
{
	return QuaternionLengthSqrV(q).x;
}

inline BvFastQuat QuaternionLengthSqrV(const BvFastQuat & q)
{
	return QuaternionDotV(q, q);
}

inline float QuaternionLength(const BvFastQuat & v)
{
	return QuaternionLengthV(v).x;
}

inline BvFastQuat QuaternionLengthV(const BvFastQuat & v)
{
	float oneOverLengthSqr = 1.0f / QuaternionLengthSqr(v);
	return BvFastQuat(v.x * oneOverLengthSqr, v.y * oneOverLengthSqr, v.z * oneOverLengthSqr, v.w * oneOverLengthSqr);
}

inline BvFastQuat QuaternionRotationAxis(const BvFastVec & v, float angle)
{
	float halfAngle = angle * 0.5f;
	float sinCos[] = { sinf(halfAngle), cosf(halfAngle) };

	return BvFastVec(v.x * sinCos[0], v.y * sinCos[0], v.z * sinCos[0], sinCos[1]);
}

inline BvFastVec QuaternionQVQC(const BvFastQuat & q, const BvFastVec & v)
{
	float w2MinusQDotQ = q.w * q.w - VectorDot(q, q);
	float twoTimesQDotV = 2.0f * VectorDot(q, v);
	float wTimesTwo = 2.0f * q.w;
	BvFastVec qCrossV = VectorCross(q, v);

	return BvFastVec(w2MinusQDotQ * v.x + twoTimesQDotV * q.x + wTimesTwo * qCrossV.x,
		w2MinusQDotQ * v.y + twoTimesQDotV * q.y + wTimesTwo * qCrossV.y,
		w2MinusQDotQ * v.z + twoTimesQDotV * q.z + wTimesTwo * qCrossV.z);
}

inline BvFastVec QuaternionQCVQ(const BvFastQuat & q, const BvFastVec & v)
{
	float w2MinusQDotQ = q.w * q.w - VectorDot(q, q);
	float twoTimesQDotV = 2.0f * VectorDot(q, v);
	float wTimesTwo = 2.0f * q.w;
	BvFastVec vCrossQ = VectorCross(v, q);

	return BvFastVec(w2MinusQDotQ * v.x + twoTimesQDotV * q.x + wTimesTwo * vCrossQ.x,
		w2MinusQDotQ * v.y + twoTimesQDotV * q.y + wTimesTwo * vCrossQ.y,
		w2MinusQDotQ * v.z + twoTimesQDotV * q.z + wTimesTwo * vCrossQ.z);
}
		
inline BvFastVec QuaternionQVQCKeenan(const BvFastQuat & q, const BvFastVec & v)
{
	return QuaternionQCVQ(q, v);
}

inline BvFastVec QuaternionQCVQKeenan(const BvFastQuat & q, const BvFastVec & v)
{
	return QuaternionQVQC(q, v);
}

inline BvFastMat QuaternionToMatrix(const BvFastQuat & q)
{
	float xy = q.x * q.y;
	float xz = q.x * q.z;
	float xw = q.x * q.w;

	float yz = q.x * q.z;
	float yw = q.x * q.w;

	float zw = q.x * q.w;

	float xx = q.x * q.x;
	float yy = q.y * q.y;
	float zz = q.z * q.z;

	BvFastMat r;
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

inline BvFastQuat QuaternionFromMatrix(const BvFastMat & m)
{
	float trace = m.r[0].x + m.r[1].y + m.r[2].z;

	BvFastQuat q;
	if (trace > 0.0f)
	{
		float s = sqrtf(trace + 1.0f) * 2.0f; // S=4*qw 
		q.w = 0.25f * s;
		q.x = (m.r[2].y - m.r[1].z) / s;
		q.y = (m.r[0].z - m.r[2].x) / s;
		q.z = (m.r[1].x - m.r[0].y) / s;
	}
	else if ((m.r[0].x > m.r[1].y) && (m.r[0].x > m.r[2].z))
	{
		float s = sqrtf(1.0f + m.r[0].x - m.r[1].y - m.r[2].z) * 2.0f; // S=4*qx 
		q.w = (m.r[2].y - m.r[1].z) / s;
		q.x = 0.25f * s;
		q.y = (m.r[0].y + m.r[1].x) / s;
		q.z = (m.r[0].z + m.r[2].x) / s;
	}
	else if (m.r[1].y > m.r[2].z)
	{
		float s = sqrtf(1.0f + m.r[1].y - m.r[0].x - m.r[2].z) * 2.0f; // S=4*qy
		q.w = (m.r[0].z - m.r[2].x) / s;
		q.x = (m.r[0].y + m.r[1].x) / s;
		q.y = 0.25f * s;
		q.z = (m.r[1].z + m.r[2].y) / s;
	}
	else
	{
		float s = sqrtf(1.0f + m.r[2].z - m.r[0].x - m.r[1].y) * 2.0f; // S=4*qz
		q.w = (m.r[1].x - m.r[0].y) / s;
		q.x = (m.r[0].z + m.r[2].x) / s;
		q.y = (m.r[1].z + m.r[2].y) / s;
		q.z = 0.25f * s;
	}

	return QuaternionNormalize(q);
}

inline BvFastQuat QuaternionSlerp(const BvFastQuat & q1, const BvFastQuat & q2, const float t, const float epsilon)
{
	float cosOmega = QuaternionDot(q1, q2);
	BvFastQuat qt(q2);
	if (cosOmega < 0.0f)
	{
		cosOmega = -cosOmega;
		qt = QuaternionConjugate(qt);
	}

	float omega = acosf(cosOmega);

	float mult1 = 1.0f - t;
	float mult2 = t;
	if (omega > epsilon)
	{
		float oneOverSinOmega = 1.0f / sinf(omega);
		mult1 = sinf(mult1 * omega) * oneOverSinOmega;
		mult2 = sinf(mult2 * omega) * oneOverSinOmega;
	}

	BvFastQuat r = q1 * mult1 + qt * mult2;

	return QuaternionNormalize(r);
}

inline float QuaternionAngle(const BvFastQuat & q)
{
	return 2.0f * acosf(q.w);
}


#endif
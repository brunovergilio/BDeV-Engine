#pragma once


#include "BvMath.h"
#include "BDeV/Core/Utils/BvUtils.h"


struct BvGeometryVertex
{
	Float3 m_Position;
	Float2 m_UV;
	Float3 m_Normal;
	Float3 m_Tangent;
	Float3 m_Bitangent;
};


namespace Internal
{
	template<typename T, typename U>
	constexpr T ConstexprPow(T base, U exponent)
	{
		static_assert(std::is_integral<U>(), "Exponent must be integral");
		return exponent == 0 ? 1 : base * ConstexprPow(base, exponent - 1);
	}

	//void CalculateTangent(u32 vertexCount, BvGeometryVertex* pVertices, u32 indexCount, u32* pIndices);
}

//template<i32 PosOffset = 0, i32 UVOffset = -1, i32 NormalOffset = -1, i32 TangentOffset = -1, i32 BitangentOffset = -1>
//void CopyVertices(BvGeometryVertex* pSrcVertices, void* pDstVertices, size_t vertexCount, size_t dstVertexStride)
//{
//	MemType mem{ pVertices };
//	for (auto i = 0; i < kBoxVertexCount; ++i, mem.pAsCharPtr += vertexStride)
//	{
//		if constexpr (PosOffset != -1)
//		{
//			*reinterpret_cast<Float3*>(mem.pAsCharPtr + PosOffset) = m_Vertices[i].m_Position;
//		}
//		if constexpr (UVOffset != -1)
//		{
//			*reinterpret_cast<Float2*>(mem.pAsCharPtr + UVOffset) = m_Vertices[i].m_UV;
//		}
//		if constexpr (NormalOffset != -1)
//		{
//			*reinterpret_cast<Float3*>(mem.pAsCharPtr + NormalOffset) = m_Vertices[i].m_Normal;
//		}
//		if constexpr (TangentOffset != -1)
//		{
//			*reinterpret_cast<Float3*>(mem.pAsCharPtr + TangentOffset) = m_Vertices[i].m_Tangent;
//		}
//		if constexpr (BitangentOffset != -1)
//		{
//			*reinterpret_cast<Float3*>(mem.pAsCharPtr + BitangentOffset) = m_Vertices[i].m_Bitangent;
//		}
//	}
//}


//class BvBox
//{
//public:
//	static constexpr u32 kTotalVertexCount = 24;
//	static constexpr u32 kTotalIndexCount = 36;
//
//	BV_DEFAULTCOPYMOVE(BvBox);
//
//	BvBox();
//	BvBox(float size);
//	BvBox(float width, float height, float depth);
//
//	BV_INLINE const auto GetVertices() const { return m_Vertices; }
//	BV_INLINE const auto GetIndices() const { return m_Indices; }
//
//private:
//	BvGeometryVertex m_Vertices[kTotalVertexCount];
//	u32 m_Indices[kTotalIndexCount];
//};
//
//
//template<u32 Subdivisions>
//class BvGeosphere
//{
//public:
//	static_assert(Subdivisions < 6);
//
//	static constexpr u32 kTotalVertexCount = 12 + 30 * (Internal::ConstexprPow(2, Subdivisions) - 1);
//	static constexpr u32 kTotalIndexCount = 20 * Internal::ConstexprPow(4, Subdivisions) * 3;
//
//	BV_DEFAULTCOPYMOVE(BvGeosphere);
//
//	BvGeosphere();
//	BvGeosphere(float radius);
//
//	BV_INLINE const auto GetVertices() const { return m_Vertices; }
//	BV_INLINE const auto GetIndices() const { return m_Indices; }
//
//private:
//	void Subdivide(u32& currVertex, u32& currIndex);
//	u32 AddMidPoint(u32& currVertex, u32 v0, u32 v1);
//	void CalculateUV(BvGeometryVertex& vertex, float invRadius);
//
//private:
//	BvGeometryVertex m_Vertices[kTotalVertexCount];
//	u32 m_Indices[kTotalIndexCount];
//};
//
//
//template<u32 Subdivisions>
//BvGeosphere<Subdivisions>::BvGeosphere()
//	: BvGeosphere(1.0f)
//{
//}
//
//
//template<u32 Subdivisions>
//BvGeosphere<Subdivisions>::BvGeosphere(float radius)
//{
//	constexpr float kX = 0.525731f;
//	constexpr float kZ = 0.850651f;
//
//	m_Vertices[0] = { { -kX, 0.0f, kZ }, {}, {}, {}, {} };
//	m_Vertices[1] = { { kX, 0.0f, kZ }, {}, {}, {}, {} };
//	m_Vertices[2] = { { -kX, 0.0f, -kZ }, {}, {}, {}, {} };
//	m_Vertices[3] = { { kX, 0.0f, -kZ }, {}, {}, {}, {} };
//	m_Vertices[4] = { { 0.0f, kZ, kX }, {}, {}, {}, {} };
//	m_Vertices[5] = { { 0.0f, kZ, -kX }, {}, {}, {}, {} };
//	m_Vertices[6] = { { 0.0f, -kZ, kX }, {}, {}, {}, {} };
//	m_Vertices[7] = { { 0.0f, -kZ, -kX }, {}, {}, {}, {} };
//	m_Vertices[8] = { { kZ, kX, 0.0f }, {}, {}, {}, {} };
//	m_Vertices[9] = { { -kZ, kX, 0.0f }, {}, {}, {}, {} };
//	m_Vertices[10] = { { kZ, -kX, 0.0f }, {}, {}, {}, {} };
//	m_Vertices[11] = { { -kZ, -kX, 0.0f }, {}, {}, {}, {} };
//
//	constexpr u32 kIndices[] =
//	{
//		0, 4, 1, 0, 9, 4, 9, 5, 4, 4, 5, 8, 4, 8, 1,
//		8, 10, 1, 8, 3, 10, 5, 3, 8, 5, 2, 3, 2, 7, 3,
//		7, 10, 3, 7, 6, 10, 7, 11, 6, 11, 0, 6, 0, 1, 6,
//		6, 1, 10, 9, 0, 11, 9, 11, 2, 9, 2, 5, 7, 2, 11
//	};
//
//	memcpy(m_Indices, kIndices, sizeof(kIndices));
//
//	u32 currVertex = 12;
//	u32 currIndex = 60;
//	for (auto i = 0; i < Subdivisions; ++i)
//	{
//		Subdivide(currVertex, currIndex);
//	}
//
//	f32 invRadius = 1.0f / radius;
//	for (auto i = 0; i < kTotalVertexCount; ++i)
//	{
//		auto& vertex = m_Vertices[i];
//		BvVec n(vertex.m_Position);
//		n.Normalize();
//		
//		BvVec p(n * radius);
//
//		vertex.m_Position = p.v3;
//		vertex.m_Normal = n.v3;
//		CalculateUV(vertex, invRadius);
//	}
//	
//	Internal::CalculateTangent(kTotalVertexCount, m_Vertices, kTotalIndexCount, m_Indices);
//}
//
//template<u32 Subdivisions>
//void BvGeosphere<Subdivisions>::Subdivide(u32& currVertex, u32& currIndex)
//{
//	u32 tmpIndices[kTotalIndexCount];
//	u32 newIndexCount = 0;
//	for (auto i = 0; i < currIndex; i += 3)
//	{
//		u32 v0 = m_Indices[i];
//		u32 v1 = m_Indices[i + 1];
//		u32 v2 = m_Indices[i + 2];
//
//		u32 a = AddMidPoint(currVertex, v0, v1);
//		u32 b = AddMidPoint(currVertex, v1, v2);
//		u32 c = AddMidPoint(currVertex, v2, v0);
//
//		// Add new triangles in clockwise order
//		tmpIndices[newIndexCount++] = v0; tmpIndices[newIndexCount++] = a; tmpIndices[newIndexCount++] = c;
//		tmpIndices[newIndexCount++] = v1; tmpIndices[newIndexCount++] = b; tmpIndices[newIndexCount++] = a;
//		tmpIndices[newIndexCount++] = v2; tmpIndices[newIndexCount++] = c; tmpIndices[newIndexCount++] = b;
//		tmpIndices[newIndexCount++] = a; tmpIndices[newIndexCount++] = b; tmpIndices[newIndexCount++] = c;
//	}
//
//	memcpy(m_Indices, tmpIndices, sizeof(u32) * newIndexCount);
//	currIndex = newIndexCount;
//}
//
//
//template<u32 Subdivisions>
//u32 BvGeosphere<Subdivisions>::AddMidPoint(u32& currVertex, u32 v0, u32 v1)
//{
//	BvVec p0(m_Vertices[v0].m_Position);
//	BvVec p1(m_Vertices[v1].m_Position);
//
//	auto midPoint = 0.5f * (p0 + p1);
//	midPoint.Normalize();
//
//	m_Vertices[currVertex].m_Position = midPoint.v3;
//
//	return currVertex++;
//}
//
//
//template<u32 Subdivisions>
//void BvGeosphere<Subdivisions>::CalculateUV(BvGeometryVertex& vertex, float invRadius)
//{
//	float u = std::atan2f(vertex.m_Position.z, vertex.m_Position.x);
//	if (u < 0.0f)
//	{
//		u += k2Pi;
//	}
//	u *= k1Div2Pi;
//
//	float v = std::acosf(vertex.m_Position.y * invRadius) * k1DivPi;
//	vertex.m_UV = Float2(u, v);
//}
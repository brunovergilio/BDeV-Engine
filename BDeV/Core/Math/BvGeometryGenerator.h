#pragma once


#include "BvMath.h"
#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/Container/BvVector.h"


class BvGeometryGenerator
{
public:
	struct Vertex
	{
		Float3 m_Position;
		Float2 m_UV;
		Float3 m_Normal;
		Float3 m_Tangent;
		Float3 m_Bitangent;

		constexpr Vertex() {}
		constexpr Vertex(const Float3& position, const Float2& uv, const Float3& normal, const Float3& tangent, const Float3& bitangent)
			: m_Position(position), m_UV(uv), m_Normal(normal), m_Tangent(tangent), m_Bitangent(bitangent) {}
	};

	struct Data
	{
		BvVector<Vertex> m_Vertices;
		BvVector<u32> m_Indices;
	};

	BvGeometryGenerator() = default;
	~BvGeometryGenerator() = default;

	void GenerateBox(f32 width = 1.0f, f32 height = 1.0f, f32 depth = 1.0f);
	void GenerateGeoSphere(f32 radius = 1.0f, u32 numSubDivisions = 1);
	void GenerateGrid(f32 xSize = 1.0f, f32 zSize = 1.0f, u32 xDiv = 1, u32 zDiv = 1);

	BV_INLINE const auto& GetData() const { return m_Data; }

private:
	void Subdivide();
	Vertex GetMidPoint(const Vertex& v0, const Vertex& v1);

private:
	Data m_Data;
};


class BvVertexUtilities
{
public:
	template<i32 PosOffset, i32 UVOffset, i32 TangentOffset, i32 BitangentOffset = -1, typename IndexType>
	static void CalculateTangent(void* pVertices, u32 vertexCount, u32 vertexStride, IndexType* pIndices, u32 indexCount);
	template<typename T>
	static void ReverseWindingOrder(T* pIndices, u32 indexCount);
};


template<i32 PosOffset, i32 UVOffset, i32 TangentOffset, i32 BitangentOffset, typename IndexType>
void BvVertexUtilities::CalculateTangent(void* pVertices, u32 vertexCount, u32 vertexStride, IndexType* pIndices, u32 indexCount)
{
	static_assert(PosOffset != -1 && UVOffset != -1 && TangentOffset != -1);

	MemType mem{ pVertices };
	for (auto i = 0; i < indexCount; i += 3)
	{
		auto pV0 = mem.pAsCharPtr + vertexStride * pIndices[i];
		auto pV1 = mem.pAsCharPtr + vertexStride * pIndices[i + 1];
		auto pV2 = mem.pAsCharPtr + vertexStride * pIndices[i + 2];

		BvVec3 p0(*reinterpret_cast<Float3*>(pV0 + PosOffset));
		BvVec3 p1(*reinterpret_cast<Float3*>(pV1 + PosOffset));
		BvVec3 p2(*reinterpret_cast<Float3*>(pV2 + PosOffset));

		BvVec2 uv0(*reinterpret_cast<Float2*>(pV0 + UVOffset));
		BvVec2 uv1(*reinterpret_cast<Float2*>(pV1 + UVOffset));
		BvVec2 uv2(*reinterpret_cast<Float2*>(pV2 + UVOffset));

		auto deltaPos1 = p1 - p0;
		auto deltaPos2 = p2 - p0;

		auto deltaUV1 = uv1 - uv0;
		auto deltaUV2 = uv2 - uv0;

		auto r = 1.0f / (deltaUV1.GetX() * deltaUV2.GetY() - deltaUV1.GetY() * deltaUV2.GetX());
		auto tangent = (deltaPos1 * deltaUV2.GetY() - deltaPos2 * deltaUV1.GetY()) * r;
		auto bitangent = (deltaPos2 * deltaUV1.GetX() - deltaPos1 * deltaUV2.GetX()) * r;

		auto& tg0 = *reinterpret_cast<Float3*>(pV0 + TangentOffset);
		auto& tg1 = *reinterpret_cast<Float3*>(pV1 + TangentOffset);
		auto& tg2 = *reinterpret_cast<Float3*>(pV2 + TangentOffset);

		BvVec3 t0(tg0);
		BvVec3 t1(tg1);
		BvVec3 t2(tg2);
		t0 += tangent;
		t1 += tangent;
		t2 += tangent;

		tg0 = t0.ToFloat();
		tg1 = t1.ToFloat();
		tg2 = t2.ToFloat();

		if constexpr (BitangentOffset != -1)
		{
			auto& bt0 = *reinterpret_cast<Float3*>(pV0 + BitangentOffset);
			auto& bt1 = *reinterpret_cast<Float3*>(pV1 + BitangentOffset);
			auto& bt2 = *reinterpret_cast<Float3*>(pV2 + BitangentOffset);

			BvVec3 b0(bt0);
			BvVec3 b1(bt1);
			BvVec3 b2(bt2);
			b0 += bitangent;
			b1 += bitangent;
			b2 += bitangent;

			bt0 = b0.ToFloat();
			bt1 = b1.ToFloat();
			bt2 = b2.ToFloat();
		}
	}

	mem.pAsVoidPtr = pVertices;
	for (auto i = 0; i < vertexCount; ++i)
	{
		auto& tg = *reinterpret_cast<Float3*>(mem.pAsCharPtr + vertexStride * i + TangentOffset);

		BvVec3 t(tg);
		t.Normalize();
		tg = t.ToFloat();
	
		if constexpr (BitangentOffset != -1)
		{
			auto& bt = *reinterpret_cast<Float3*>(mem.pAsCharPtr + vertexStride * i + BitangentOffset);

			BvVec3 b(bt);
			b.Normalize();
			bt = b.ToFloat();
		}
	}
}


template<typename T>
void BvVertexUtilities::ReverseWindingOrder(T* pIndices, u32 indexCount)
{
	for (auto i = 0u; i < indexCount; ++i)
	{
		std::swap(pIndices[i], pIndices[i + 2]);
	}
}
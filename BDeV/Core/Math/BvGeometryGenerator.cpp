#include "BvGeometryGenerator.h"
#include <cmath>


void BvGeometryGenerator::GenerateBox(f32 width, f32 height, f32 depth)
{
	constexpr u32 kTotalVertexCount = 24;
	constexpr u32 kTotalIndexCount = 36;

	m_Data.m_Vertices.Resize(kTotalVertexCount);
	m_Data.m_Indices.Resize(kTotalIndexCount);

	f32 w = width * 0.5f;
	f32 h = height * 0.5f;
	f32 d = depth * 0.5f;

	auto& v = m_Data.m_Vertices;
	v[0] = Vertex({ -w, -h, -d }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
	v[1] = Vertex({ -w, +h, -d }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
	v[2] = Vertex({ +w, +h, -d }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
	v[3] = Vertex({ +w, -h, -d }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });

	// Fill in the back face vertex data.
	v[4] = Vertex({ -w, -h, +d }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
	v[5] = Vertex({ +w, -h, +d }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
	v[6] = Vertex({ +w, +h, +d }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
	v[7] = Vertex({ -w, +h, +d }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });

	// Fill in the top face vertex data.
	v[8] =  Vertex({ -w, +h, -d }, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
	v[9] =  Vertex({ -w, +h, +d }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
	v[10] = Vertex({ +w, +h, +d }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
	v[11] = Vertex({ +w, +h, -d }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });

	// Fill in the bottom face vertex data.
	v[12] = Vertex({ -w, -h, -d }, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
	v[13] = Vertex({ +w, -h, -d }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
	v[14] = Vertex({ +w, -h, +d }, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
	v[15] = Vertex({ -w, -h, +d }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });

	// Fill in the left face vertex data.
	v[16] = Vertex({ -w, -h, +d }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f });
	v[17] = Vertex({ -w, +h, +d }, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f });
	v[18] = Vertex({ -w, +h, -d }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f });
	v[19] = Vertex({ -w, -h, -d }, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f });

	// Fill in the right face vertex data.
	v[20] = Vertex({ +w, -h, -d }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f });
	v[21] = Vertex({ +w, +h, -d }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f });
	v[22] = Vertex({ +w, +h, +d }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f });
	v[23] = Vertex({ +w, -h, +d }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f });

	for (auto i = 0u, j = 0u; i < kTotalIndexCount; i += 6, ++j)
	{
		m_Data.m_Indices[i + 0] = 0 + j * 4;
		m_Data.m_Indices[i + 1] = 1 + j * 4;
		m_Data.m_Indices[i + 2] = 2 + j * 4;
		m_Data.m_Indices[i + 3] = 2 + j * 4;
		m_Data.m_Indices[i + 4] = 3 + j * 4;
		m_Data.m_Indices[i + 5] = 0 + j * 4;
	}
}


void BvGeometryGenerator::GenerateGeoSphere(f32 radius, u32 numSubDivisions)
{
	numSubDivisions = std::min(numSubDivisions, 6u);

	m_Data.m_Vertices.Resize(12);
	m_Data.m_Indices.Resize(60);

	constexpr f32 kX = 0.525731f;
	constexpr f32 kZ = 0.850651f;
	
	m_Data.m_Vertices[0] = { { -kX, 0.0f, kZ }, {}, {}, {}, {} };
	m_Data.m_Vertices[1] = { { kX, 0.0f, kZ }, {}, {}, {}, {} };
	m_Data.m_Vertices[2] = { { -kX, 0.0f, -kZ }, {}, {}, {}, {} };
	m_Data.m_Vertices[3] = { { kX, 0.0f, -kZ }, {}, {}, {}, {} };
	m_Data.m_Vertices[4] = { { 0.0f, kZ, kX }, {}, {}, {}, {} };
	m_Data.m_Vertices[5] = { { 0.0f, kZ, -kX }, {}, {}, {}, {} };
	m_Data.m_Vertices[6] = { { 0.0f, -kZ, kX }, {}, {}, {}, {} };
	m_Data.m_Vertices[7] = { { 0.0f, -kZ, -kX }, {}, {}, {}, {} };
	m_Data.m_Vertices[8] = { { kZ, kX, 0.0f }, {}, {}, {}, {} };
	m_Data.m_Vertices[9] = { { -kZ, kX, 0.0f }, {}, {}, {}, {} };
	m_Data.m_Vertices[10] = { { kZ, -kX, 0.0f }, {}, {}, {}, {} };
	m_Data.m_Vertices[11] = { { -kZ, -kX, 0.0f }, {}, {}, {}, {} };
	
	constexpr u32 kIndices[] =
	{
		1, 4, 0, 4, 9, 0, 4, 5, 9, 8, 5, 4, 1, 8, 4,    
		1, 10, 8, 10, 3, 8, 8, 3, 5, 3, 2, 5, 3, 7, 2,    
		3, 10, 7, 10, 6, 7, 6, 11, 7, 6, 0, 11, 6, 1, 0, 
		10, 1, 6, 11, 0, 9, 2, 11, 9, 5, 2, 9, 11, 2, 7
	};
	
	memcpy(m_Data.m_Indices.Data(), kIndices, sizeof(kIndices));

	for (auto i = 0; i < numSubDivisions; ++i)
	{
		Subdivide();
	}

	f32 invRadius = 1.0f / radius;
	for (auto i = 0; i < m_Data.m_Vertices.Size(); ++i)
	{
		auto& vertex = m_Data.m_Vertices[i];
		auto p = XMLoadFloat3(&vertex.m_Position);
		auto n = XMVector3Normalize(p);
		p = (n * radius);
	
		XMStoreFloat3(&vertex.m_Position, p);
		XMStoreFloat3(&vertex.m_Normal, n);
		
		f32 u = std::atan2f(vertex.m_Position.z, vertex.m_Position.x);
		if (u < 0.0f)
		{
			u += k2Pi;
		}
		
		f32 v = std::acosf(vertex.m_Position.y * invRadius);
		vertex.m_UV = XMFLOAT2(u * k1Div2Pi, v * k1DivPi);

		f32 su = sinf(u);
		f32 sv = sinf(v);
		f32 cu = cosf(u);
		f32 cv = cosf(v);

		vertex.m_Tangent = XMFLOAT3(-radius * su * sv, 0.0f, radius * cu * sv);
		vertex.m_Bitangent = XMFLOAT3(radius * cu * cv, radius * su * cv, -radius * sv);
	}
}


void BvGeometryGenerator::GenerateGrid(f32 xSize, f32 zSize, u32 xDiv, u32 zDiv)
{
	XMFLOAT3 normal{ 0.0f, 1.0f, 0.0f };
	XMFLOAT3 tangent{ 1.0f, 0.0f, 0.0f };
	XMFLOAT3 bitangent{ 0.0f, 0.0f, -1.0f };

	f32 xStep = xSize / f32(i32(xDiv));
	f32 zStep = zSize / f32(i32(zDiv));

	f32 xHalf = xSize * 0.5f;
	f32 zHalf = zSize * 0.5f;

	u32 xLine = xDiv + 1;
	u32 zLine = zDiv + 1;
	auto& v = m_Data.m_Vertices;
	v.Resize(xLine * zLine);
	for (auto z = 0; z < zLine; ++z)
	{
		f32 z0 = -zHalf + z * zStep;
		u32 baseIndex = xLine * z;

		for (auto x = 0; x < xLine; ++x)
		{
			f32 x0 = -xHalf + x * xStep;

			v[baseIndex + x] = { { x0, 0.0, z0 }, { x / f32(i32(xDiv)), z / f32(i32(zDiv)) }, normal, tangent, bitangent };
		}
	}

	auto& indices = m_Data.m_Indices;
	indices.Resize(xDiv * zDiv * 6);
	for (auto z = 0, i = 0; z < zDiv; ++z)
	{
		u32 baseIndex0 = xLine * z;
		u32 baseIndex1 = xLine * (z + 1);
		for (auto x = 0; x < xDiv; ++x)
		{
			indices[i++] = baseIndex0 + x;
			indices[i++] = baseIndex0 + x + 1;
			indices[i++] = baseIndex1 + x + 1;

			indices[i++] = baseIndex1 + x + 1;
			indices[i++] = baseIndex1 + x;
			indices[i++] = baseIndex0 + x;
		}
	}
}


//void BvGeometryGenerator::CopyVertices(void* pVertices, size_t count, size_t stride,
//	i32 posOffset, i32 uvOffset, i32 normalOffset, i32 tangentOffset, i32 bitangentOffset)
//{
//	MemType mem{ pVertices };
//	for (auto i = 0; i < count; ++i, mem.pAsCharPtr += stride)
//	{
//		auto& vertex = m_Data.m_Vertices[i];
//		if (posOffset != -1)
//		{
//			*reinterpret_cast<XMFLOAT3*>(mem.pAsCharPtr + posOffset) = vertex.m_Position;
//		}
//		if (uvOffset != -1)
//		{
//			*reinterpret_cast<XMFLOAT2*>(mem.pAsCharPtr + uvOffset) = vertex.m_UV;
//		}
//		if (normalOffset != -1)
//		{
//			*reinterpret_cast<XMFLOAT3*>(mem.pAsCharPtr + normalOffset) = vertex.m_Normal;
//		}
//		if (tangentOffset != -1)
//		{
//			*reinterpret_cast<XMFLOAT3*>(mem.pAsCharPtr + tangentOffset) = vertex.m_Tangent;
//		}
//		if (bitangentOffset != -1)
//		{
//			*reinterpret_cast<XMFLOAT3*>(mem.pAsCharPtr + bitangentOffset) = vertex.m_Bitangent;
//		}
//	}
//}


void BvGeometryGenerator::Subdivide()
{
	Data tmpData = m_Data;
	m_Data.m_Vertices.Clear();
	m_Data.m_Indices.Clear();

	auto& indices = tmpData.m_Indices;
	auto& vertices = tmpData.m_Vertices;
	for (auto i = 0; i < indices.Size(); i += 3)
	{
		auto& v0 = vertices[indices[i]];
		auto& v1 = vertices[indices[i + 1]];
		auto& v2 = vertices[indices[i + 2]];
	
		auto a = GetMidPoint(v0, v1);
		auto b = GetMidPoint(v1, v2);
		auto c = GetMidPoint(v2, v0);

		m_Data.m_Vertices.EmplaceBack(v0);
		m_Data.m_Vertices.EmplaceBack(v1);
		m_Data.m_Vertices.EmplaceBack(v2);
		m_Data.m_Vertices.EmplaceBack(a);
		m_Data.m_Vertices.EmplaceBack(b);
		m_Data.m_Vertices.EmplaceBack(c);
	
		// Add new triangles in clockwise order
		m_Data.m_Indices.EmplaceBack(i * 2 + 0); m_Data.m_Indices.EmplaceBack(i * 2 + 3); m_Data.m_Indices.EmplaceBack(i * 2 + 5);
		m_Data.m_Indices.EmplaceBack(i * 2 + 3); m_Data.m_Indices.EmplaceBack(i * 2 + 4); m_Data.m_Indices.EmplaceBack(i * 2 + 5);
		m_Data.m_Indices.EmplaceBack(i * 2 + 5); m_Data.m_Indices.EmplaceBack(i * 2 + 4); m_Data.m_Indices.EmplaceBack(i * 2 + 2);
		m_Data.m_Indices.EmplaceBack(i * 2 + 3); m_Data.m_Indices.EmplaceBack(i * 2 + 1); m_Data.m_Indices.EmplaceBack(i * 2 + 4);
	}
}


BvGeometryGenerator::Vertex BvGeometryGenerator::GetMidPoint(const Vertex& v0, const Vertex& v1)
{
	auto p0 = XMLoadFloat3(&v0.m_Position);
	auto p1 = XMLoadFloat3(&v1.m_Position);

	auto uv0 = XMLoadFloat2(&v0.m_UV);
	auto uv1 = XMLoadFloat2(&v1.m_UV);

	auto n0 = XMLoadFloat3(&v0.m_Normal);
	auto n1 = XMLoadFloat3(&v1.m_Normal);

	auto t0 = XMLoadFloat3(&v0.m_Tangent);
	auto t1 = XMLoadFloat3(&v1.m_Tangent);

	auto b0 = XMLoadFloat3(&v0.m_Bitangent);
	auto b1 = XMLoadFloat3(&v1.m_Bitangent);

	Vertex v;
	XMStoreFloat3(&v.m_Position, 0.5f * (p0 + p1));
	XMStoreFloat2(&v.m_UV, 0.5f * (uv0 + uv1));
	XMStoreFloat3(&v.m_Normal, XMVector3Normalize(0.5f * (n0 + n1)));
	XMStoreFloat3(&v.m_Tangent, XMVector3Normalize(0.5f * (t0 + t1)));
	XMStoreFloat3(&v.m_Bitangent, XMVector3Normalize(0.5f * (b0 + b1)));

	return v;
}
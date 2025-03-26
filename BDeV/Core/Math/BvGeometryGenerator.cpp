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

	// Clockwise order, left-handed
	// 
	// Front face
	m_Data.m_Vertices[0] = { {  w, -h, -d }, { 0.0f, 1.0f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Data.m_Vertices[1] = { { -w, -h, -d }, { 1.0f, 1.0f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Data.m_Vertices[2] = { { -w,  h, -d }, { 1.0f, 0.0f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Data.m_Vertices[3] = { {  w,  h, -d }, { 0.0f, 0.0f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };

	// Back face
	m_Data.m_Vertices[4] = { { -w, -h,  d }, { 0.0f, 1.0f }, { 0.0f,  0.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Data.m_Vertices[5] = { {  w, -h,  d }, { 1.0f, 1.0f }, { 0.0f,  0.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Data.m_Vertices[6] = { {  w,  h,  d }, { 1.0f, 0.0f }, { 0.0f,  0.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Data.m_Vertices[7] = { { -w,  h,  d }, { 0.0f, 0.0f }, { 0.0f,  0.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f, 0.0f } };

	// Left face
	m_Data.m_Vertices[8] = { { -w, -h, -d }, { 0.0f, 1.0f }, { -1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Data.m_Vertices[9] = { { -w, -h,  d }, { 1.0f, 1.0f }, { -1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Data.m_Vertices[10] = { { -w,  h,  d }, { 1.0f, 0.0f }, { -1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Data.m_Vertices[11] = { { -w,  h, -d }, { 0.0f, 0.0f }, { -1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } };

	// Right face
	m_Data.m_Vertices[12] = { {  w, -h,  d }, { 0.0f, 1.0f }, { 1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Data.m_Vertices[13] = { {  w, -h, -d }, { 1.0f, 1.0f }, { 1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Data.m_Vertices[14] = { {  w,  h, -d }, { 1.0f, 0.0f }, { 1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Data.m_Vertices[15] = { {  w,  h,  d }, { 0.0f, 0.0f }, { 1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } };

	// Top face
	m_Data.m_Vertices[16] = { { -w,  h,  d }, { 0.0f, 1.0f }, { 0.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
	m_Data.m_Vertices[17] = { {  w,  h,  d }, { 1.0f, 1.0f }, { 0.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
	m_Data.m_Vertices[18] = { {  w,  h, -d }, { 1.0f, 0.0f }, { 0.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
	m_Data.m_Vertices[19] = { { -w,  h, -d }, { 0.0f, 0.0f }, { 0.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

	// Bottom face
	m_Data.m_Vertices[20] = { { -w, -h, -d }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
	m_Data.m_Vertices[21] = { {  w, -h, -d }, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
	m_Data.m_Vertices[22] = { {  w, -h,  d }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
	m_Data.m_Vertices[23] = { { -w, -h,  d }, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };

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
		auto p = Load3(vertex.m_Position.v);
		auto n = Vector3Normalize(p);
		p = (n * radius);
	
		Store3(p, vertex.m_Position.v);
		Store3(n, vertex.m_Normal.v);
		
		f32 u = std::atan2f(vertex.m_Position.z, vertex.m_Position.x);
		if (u < 0.0f)
		{
			u += k2Pi;
		}
		
		f32 v = std::acosf(vertex.m_Position.y * invRadius);
		vertex.m_UV = Float2(u * k1Div2Pi, v * k1DivPi);

		f32 su = sinf(u);
		f32 sv = sinf(v);
		f32 cu = cosf(u);
		f32 cv = cosf(v);

		vertex.m_Tangent = Float3(-radius * su * sv, 0.0f, radius * cu * sv);
		vertex.m_Bitangent = Float3(radius * cu * cv, radius * su * cv, -radius * sv);
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
//			*reinterpret_cast<Float3*>(mem.pAsCharPtr + posOffset) = vertex.m_Position;
//		}
//		if (uvOffset != -1)
//		{
//			*reinterpret_cast<Float2*>(mem.pAsCharPtr + uvOffset) = vertex.m_UV;
//		}
//		if (normalOffset != -1)
//		{
//			*reinterpret_cast<Float3*>(mem.pAsCharPtr + normalOffset) = vertex.m_Normal;
//		}
//		if (tangentOffset != -1)
//		{
//			*reinterpret_cast<Float3*>(mem.pAsCharPtr + tangentOffset) = vertex.m_Tangent;
//		}
//		if (bitangentOffset != -1)
//		{
//			*reinterpret_cast<Float3*>(mem.pAsCharPtr + bitangentOffset) = vertex.m_Bitangent;
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
	auto p0 = Load3(v0.m_Position.v);
	auto p1 = Load3(v1.m_Position.v);

	auto uv0 = Load2(v0.m_UV.v);
	auto uv1 = Load2(v1.m_UV.v);

	auto n0 = Load3(v0.m_Normal.v);
	auto n1 = Load3(v1.m_Normal.v);

	auto t0 = Load3(v0.m_Tangent.v);
	auto t1 = Load3(v1.m_Tangent.v);

	auto b0 = Load3(v0.m_Bitangent.v);
	auto b1 = Load3(v1.m_Bitangent.v);

	Vertex v;
	Store3(0.5f * (p0 + p1), v.m_Position.v);
	Store2(0.5f * (uv0 + uv1), v.m_UV.v);
	Store3(Vector3Normalize(0.5f * (n0 + n1)), v.m_Normal.v);
	Store3(Vector3Normalize(0.5f * (t0 + t1)), v.m_Tangent.v);
	Store3(Vector3Normalize(0.5f * (b0 + b1)), v.m_Bitangent.v);

	return v;
}
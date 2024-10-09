#include "BvGeometryGenerator.h"


void Internal::CalculateTangent(u32 vertexCount, BvGeometryVertex* pVertices, u32 indexCount, u32* pIndices)
{
	for (auto i = 0; i < indexCount; i += 3)
	{
		auto& v0 = pVertices[pIndices[i]];
		auto& v1 = pVertices[pIndices[i + 1]];
		auto& v2 = pVertices[pIndices[i + 2]];

		BvVec p0(v0.m_Position), p1(v1.m_Position), p2(v2.m_Position);
		BvVec uv0(v0.m_UV), uv1(v1.m_UV), uv2(v2.m_UV);

		auto deltaPos1 = p1 - p0;
		auto deltaPos2 = p2 - p0;

		auto deltaUV1 = uv1 - uv0;
		auto deltaUV2 = uv2 - uv0;

		f32 r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		auto tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		auto bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

		BvVec t0(v0.m_Tangent);
		BvVec t1(v1.m_Tangent);
		BvVec t2(v2.m_Tangent);
		t0 += tangent;
		t1 += tangent;
		t2 += tangent;

		v0.m_Tangent = t0.v3;
		v1.m_Tangent = t1.v3;
		v2.m_Tangent = t2.v3;

		BvVec b0(v0.m_Bitangent);
		BvVec b1(v1.m_Bitangent);
		BvVec b2(v2.m_Bitangent);
		b0 += bitangent;
		b1 += bitangent;
		b2 += bitangent;

		v0.m_Bitangent = b0.v3;
		v1.m_Bitangent = b1.v3;
		v2.m_Bitangent = b2.v3;
	}

	for (auto i = 0; i < vertexCount; ++i)
	{
		BvVec t(pVertices[i].m_Tangent);
		t.Normalize();
		pVertices[i].m_Tangent = t.v3;

		BvVec b(pVertices[i].m_Bitangent);
		b.Normalize();
		pVertices[i].m_Bitangent = b.v3;
	}
}


BvBox::BvBox()
	: BvBox(1.0f)
{
}


BvBox::BvBox(float size)
	: BvBox(size, size, size)
{
}


BvBox::BvBox(float width, float height, float depth)
{
	float w = width * 0.5f;
	float h = height * 0.5f;
	float d = depth * 0.5f;

	// Clockwise order, left-handed
	// 
	// Front face
	m_Vertices[0] = { {  w, -h, -d }, { 0.0f, 1.0f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Vertices[1] = { { -w, -h, -d }, { 1.0f, 1.0f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Vertices[2] = { { -w,  h, -d }, { 1.0f, 0.0f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Vertices[3] = { {  w,  h, -d }, { 0.0f, 0.0f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };

	// Back face
	m_Vertices[4] = { { -w, -h,  d }, { 0.0f, 1.0f }, { 0.0f,  0.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Vertices[5] = { {  w, -h,  d }, { 1.0f, 1.0f }, { 0.0f,  0.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Vertices[6] = { {  w,  h,  d }, { 1.0f, 0.0f }, { 0.0f,  0.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Vertices[7] = { { -w,  h,  d }, { 0.0f, 0.0f }, { 0.0f,  0.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f, 0.0f } };

	// Left face
	m_Vertices[8] =  { { -w, -h, -d }, { 0.0f, 1.0f }, { -1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Vertices[9] =  { { -w, -h,  d }, { 1.0f, 1.0f }, { -1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Vertices[10] = { { -w,  h,  d }, { 1.0f, 0.0f }, { -1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Vertices[11] = { { -w,  h, -d }, { 0.0f, 0.0f }, { -1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } };

	// Right face
	m_Vertices[12] = { {  w, -h,  d }, { 0.0f, 1.0f }, { 1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Vertices[13] = { {  w, -h, -d }, { 1.0f, 1.0f }, { 1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Vertices[14] = { {  w,  h, -d }, { 1.0f, 0.0f }, { 1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } };
	m_Vertices[15] = { {  w,  h,  d }, { 0.0f, 0.0f }, { 1.0f,  0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } };

	// Top face
	m_Vertices[16] = { { -w,  h,  d }, { 0.0f, 1.0f }, { 0.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
	m_Vertices[17] = { {  w,  h,  d }, { 1.0f, 1.0f }, { 0.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
	m_Vertices[18] = { {  w,  h, -d }, { 1.0f, 0.0f }, { 0.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
	m_Vertices[19] = { { -w,  h, -d }, { 0.0f, 0.0f }, { 0.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

	// Bottom face
	m_Vertices[20] = { { -w, -h, -d }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
	m_Vertices[21] = { {  w, -h, -d }, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
	m_Vertices[22] = { {  w, -h,  d }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
	m_Vertices[23] = { { -w, -h,  d }, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };

	for (auto i = 0, j = 0; i < kTotalIndexCount; i += 6, ++j)
	{
		m_Indices[i + 0] = 0 + j * 4;
		m_Indices[i + 1] = 1 + j * 4;
		m_Indices[i + 2] = 2 + j * 4;
		m_Indices[i + 3] = 2 + j * 4;
		m_Indices[i + 4] = 3 + j * 4;
		m_Indices[i + 5] = 0 + j * 4;
	}
}
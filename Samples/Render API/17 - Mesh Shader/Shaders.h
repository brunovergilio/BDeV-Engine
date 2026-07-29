#pragma once


#include <string_view>


#if defined(USE_D3D12)
constexpr const char* g_pASShader =
R"raw(
struct CubeData
{
    float3 position;
    float scale;
};

StructuredBuffer<CubeData> cubes : register(t0);
RWStructuredBuffer<uint> visibleIndices : register(u1);

cbuffer PushConstants : register(b2)
{
	row_major float4x4 viewProj;
	float4 cameraPosAndTime;
};

// Max payload limit is 16KB. This structure passes visibility indices down to the mesh shader
struct TaskPayload
{
	uint visibleIndices[64];
};

groupshared uint visibleCount;
groupshared uint localVisibleIndices[64];

// Extract frustum planes from view-projection matrix
float4 getPlane(float4 row1, float4 row2, bool add)
{
	return add ? (row1 + row2) : (row1 - row2);
}

bool sphereInFrustum(float3 center, float radius)
{
	matrix m = viewProj;

	float4 planes[6];
	planes[0] = getPlane(m[3], m[0], false); // Left
	planes[1] = getPlane(m[3], m[0], true);  // Right
	planes[2] = getPlane(m[3], m[1], false); // Bottom
	planes[3] = getPlane(m[3], m[1], true);  // Top
	planes[4] = getPlane(m[3], m[2], false); // Near
	planes[5] = getPlane(m[3], m[2], true);  // Far

	for (int i = 0; i < 6; ++i)
	{
		float4 p = planes[i];
		float d = dot(p.xyz, center) + p.w;
		if (d < -radius) return false;
	}
	return true;
}

[numthreads(64, 1, 1)]
void main(uint3 gid : SV_DispatchThreadID, uint3 gtid : SV_GroupThreadID)
{
	uint index = gid.x;

	if (gtid.x == 0)
	{
		visibleCount = 0;
	}

	GroupMemoryBarrierWithGroupSync();
	//GroupMemoryBarrier();
	//AllMemoryBarrierWithGroupSync();

	// Check buffer bounds safely since HLSL StructuredBuffers don't provide an inline .length() operator
	uint numCubes, stroke;
	cubes.GetDimensions(numCubes, stroke);

	bool isVisible = false;
	if (index < numCubes)
	{
		CubeData cube = cubes[index];
		float radius = cube.scale * 0.5f * sqrt(3.0f);
		if (sphereInFrustum(cube.position, radius))
		{
			isVisible = true;
		}
	}

	if (isVisible)
	{
		uint writeIndex;
		InterlockedAdd(visibleCount, 1, writeIndex);
		localVisibleIndices[writeIndex] = index;
	}
	GroupMemoryBarrierWithGroupSync();

	TaskPayload payload;
	if (gtid.x == 0)
	{
		for (uint i = 0; i < visibleCount; ++i)
		{
			payload.visibleIndices[i] = localVisibleIndices[i];

			// Also write out globally to match your original RWBuffer logic
			visibleIndices[i] = localVisibleIndices[i];
		}
	}

	GroupMemoryBarrierWithGroupSync();

	// Dispatches mesh shader wave groups
	DispatchMesh(visibleCount, 1, 1, payload);
}
)raw";
constexpr auto g_ASSize = std::char_traits<char>::length(g_pASShader);

constexpr const char* g_pMSShader =
R"raw(
struct CubeData
{
    float3 position;
    float scale;
};

StructuredBuffer<CubeData> cubes : register(t0);

cbuffer PushConstants : register(b2)
{
	row_major float4x4 viewProj;
	float3 cameraPos;
	float time;
};

struct TaskPayload
{
	uint visibleIndices[64];
};

struct MeshVertexOutput
{
	float4 position : SV_Position;
	float4 color : COLOR;
};

[numthreads(1, 1, 1)] // Operates on single linear groups matching your WorkGroupID mapping
[outputtopology("triangle")]
void main(
	in payload TaskPayload payload,
	uint3 gid : SV_GroupID,
	out vertices MeshVertexOutput verts[8],
	out indices uint3 tris[12]) 
{
	// Fix output size attributes explicitly up front
	SetMeshOutputCounts(8, 12);

	uint visibleIndex = gid.x;
	CubeData cube = cubes[payload.visibleIndices[visibleIndex]];

	float3 offsets[8] =
	{
		float3(-1,-1,-1), float3(-1,1,-1), float3(1,1,-1), float3(1,-1,-1),
		float3(-1,-1, 1), float3(-1,1, 1), float3(1,1, 1), float3(1,-1, 1)
	};

	for (uint i = 0; i < 8; ++i)
	{
		float3 pos = cube.position + offsets[i] * cube.scale * 0.5f;
		verts[i].position = mul(float4(pos, 1.0f), viewProj);
		verts[i].color = float4(0.2f, 0.6f, 1.0f, 1.0f);
	}

	uint indicesRaw[36] =
	{
		0, 1, 2,  0, 2, 3, // Back
		4, 6, 5,  4, 7, 6, // Front
		0, 5, 1,  0, 4, 5, // Left
		3, 2, 6,  3, 6, 7, // Right
		1, 5, 6,  1, 6, 2, // Top
		0, 3, 7,  0, 7, 4  // Bottom
	};

	for (uint j = 0; j < 12; ++j)
	{
		tris[j] = uint3(indicesRaw[j * 3], indicesRaw[j * 3 + 1], indicesRaw[j * 3 + 2]);
	}
}
)raw";
constexpr auto g_MSSize = std::char_traits<char>::length(g_pMSShader);

constexpr const char* g_pPSShader =
R"raw(
struct VSOutput
{
	float4 position : SV_Position;
	float4 outColor : COLOR;
};

float4 main(VSOutput input) : SV_Target
{
	return input.outColor;
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#else
constexpr const char* g_pASShader =
R"raw(
#version 460
#extension GL_EXT_mesh_shader : require

layout(local_size_x = 64) in;

struct CubeData {
    vec3 position;
    float scale;
};

layout(binding = 0) readonly buffer CubeBuffer {
    CubeData cubes[];
};

layout(binding = 1) buffer VisibleBuffer {
    uint visibleIndices[];
};

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    vec3 cameraPos;
    float time;
} pc;

shared uint visibleCount;

// Extract frustum planes from view-projection matrix
vec4 getPlane(vec4 row1, vec4 row2, bool add) {
    return add ? (row1 + row2) : (row1 - row2);
}

bool sphereInFrustum(vec3 center, float radius) {
    mat4 m = pc.viewProj;

    vec4 planes[6];
    planes[0] = getPlane(m[3], m[0], false); // Left
    planes[1] = getPlane(m[3], m[0], true);  // Right
    planes[2] = getPlane(m[3], m[1], false); // Bottom
    planes[3] = getPlane(m[3], m[1], true);  // Top
    planes[4] = getPlane(m[3], m[2], false); // Near
    planes[5] = getPlane(m[3], m[2], true);  // Far

    for (int i = 0; i < 6; ++i) {
        vec4 p = planes[i];
        float d = dot(p.xyz, center) + p.w;
        if (d < -radius) return false;
    }
    return true;
}

void main() {
    uint index = gl_GlobalInvocationID.x;

    if (gl_LocalInvocationIndex == 0) visibleCount = 0;
    memoryBarrierShared();
    barrier();

    if (index >= cubes.length()) return;

    CubeData cube = cubes[index];
    float radius = cube.scale * 0.5 * sqrt(3.0); // bounding sphere

    if (sphereInFrustum(cube.position, radius))
	{
        uint writeIndex = atomicAdd(visibleCount, 1);
        visibleIndices[writeIndex] = index;
    }

    barrier();

    if (gl_LocalInvocationID.x == 0)
	{
		if (visibleCount > 0)
		{
			EmitMeshTasksEXT(visibleCount, 1, 1);
		}
    }
}
)raw";
constexpr auto g_ASSize = std::char_traits<char>::length(g_pASShader);

constexpr const char* g_pMSShader =
R"raw(
#version 460
#extension GL_EXT_mesh_shader : require

layout(triangles) out;
layout(max_vertices = 8, max_primitives = 12) out;

struct CubeData {
    vec3 position;
    float scale;
};

layout(binding = 0) readonly buffer CubeBuffer {
    CubeData cubes[];
};

layout(binding = 1) readonly buffer VisibleBuffer {
    uint visibleIndices[];
};

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    vec3 cameraPos;
    float time;
} pc;

layout(location = 0) out vec4 outColor[];

void main() {
    uint visibleIndex = gl_WorkGroupID.x;
    CubeData cube = cubes[visibleIndices[visibleIndex]];

    // Build a small cube from scratch
    vec3 offsets[8] = vec3[](
        vec3(-1,-1,-1), vec3(-1,1,-1), vec3(1,1,-1), vec3(1,-1,-1),
        vec3(-1,-1, 1), vec3(-1,1, 1), vec3(1,1, 1), vec3(1,-1, 1)
    );

    for (uint i = 0; i < 8; ++i) {
        vec3 pos = cube.position + offsets[i] * cube.scale * 0.5;
        gl_MeshVerticesEXT[i].gl_Position = pc.viewProj * vec4(pos, 1.0);
        outColor[i] = vec4(0.2, 0.6, 1.0, 1.0);
    }

	uint tris[36] = uint[](
	    // Back face (Z-)
	    0, 1, 2,  0, 2, 3,
	
	    // Front face (Z+)
	    4, 6, 5,  4, 7, 6,
	
	    // Left face (X-)
	    0, 5, 1,  0, 4, 5,
	
	    // Right face (X+)
	    3, 2, 6,  3, 6, 7,
	
	    // Top face (Y+)
	    1, 5, 6,  1, 6, 2,
	
	    // Bottom face (Y-)
	    0, 3, 7,  0, 7, 4
	);

    for (uint i = 0; i < 12; ++i) {
        gl_PrimitiveTriangleIndicesEXT[i] = uvec3(tris[i*3], tris[i*3+1], tris[i*3+2]);
    }

    SetMeshOutputsEXT(8, 12);
}
)raw";
constexpr auto g_MSSize = std::char_traits<char>::length(g_pMSShader);

constexpr const char* g_pPSShader =
R"raw(
#version 450

layout (location = 0) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main()
{
	outColor = inColor;
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#endif
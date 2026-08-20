#pragma once


#include <string_view>


#if defined(USE_D3D12)
constexpr const char* g_pRGenShader =
R"raw(
RWTexture2D<float4> outputImage : register(u0);
RaytracingAccelerationStructure topLevelAS : register(t1);

struct RayData
{
	row_major float4x4 viewInv;
	row_major float4x4 projInv;
};
ConstantBuffer<RayData> rayData : register(b2);

struct RayPayload
{
	float4 color;
};

[shader("raygeneration")]
void main()
{
	const uint2 launchID = DispatchRaysIndex().xy;
	const uint2 launchSize = DispatchRaysDimensions().xy;

	const float2 pixelCenter = float2(launchID) + float2(0.5f, 0.5f);
	const float2 inUV = pixelCenter / float2(launchSize);
	float2 d = inUV * 2.0f - 1.0f;
	d.y = -d.y;

	// Direct translation of your vector projection transformations
	float4 origin = mul(float4(0.0f, 0.0f, 0.0f, 1.0f), rayData.viewInv);
	float4 target = mul(float4(d.x, d.y, 1.0f, 1.0f), rayData.projInv);
	float4 direction = mul(float4(normalize(target.xyz), 0.0f), rayData.viewInv);

	float tmin = 0.001f;
	float tmax = 10000.0f;

	RayPayload payload;
	payload.color = float4(d, 0.0f, 1.0f);

	RayDesc ray;
	ray.Origin	= origin.xyz;
	ray.TMin		= tmin;
	ray.Direction = direction.xyz;
	ray.TMax		= tmax;
	//ray.Origin		= float3(d.x, d.y, -1.0f); // Position the ray starting plane slightly back
	//ray.TMin		= 0.001f;
	//ray.Direction	= float3(0.0f, 0.0f, 1.0f);  // Shoot straight forward down the Z-axis
	//ray.TMax		= 10000.0f;

	// Maps gl_RayFlagsOpaqueEXT to RAY_FLAG_FORCE_OPAQUE
	TraceRay(
		topLevelAS,
		RAY_FLAG_FORCE_OPAQUE,
		0xFF,
		0,
		0,
		0,
		ray,
		payload
	);

	outputImage[launchID] = payload.color;
}
)raw";
constexpr auto g_RGenSize = std::char_traits<char>::length(g_pRGenShader);


constexpr const char* g_pRMissShader =
R"raw(
struct RayPayload
{
	float4 color;
};

cbuffer PC : register(b3)
{
	float3 pcColor;
};

float3 verticalGradient(float3 topColor, float uvY)
{
	return topColor * uvY;
}

[shader("miss")]
void main(inout RayPayload payload)
{
	float2 uv = payload.color.xy;
	float3 color = verticalGradient(pcColor, 1.0f - uv.y);
	payload.color = float4(color, 1.0f);
}
)raw";
constexpr auto g_RMissSize = std::char_traits<char>::length(g_pRMissShader);


constexpr const char* g_pRCHitShader =
R"raw(
struct RayPayload {
    float4 color;
};

struct ShadowPayload {
    bool inShadow;
};

struct Vertex {
    float3 pos;
    float2 uv;
    float3 normal;
};

RaytracingAccelerationStructure topLevelAS : register(t1, space0);

StructuredBuffer<Vertex> vertices : register(t0, space1);
StructuredBuffer<uint> indices : register(t1, space1);

cbuffer HitData : register(b2, space1)
{
    row_major float4x4 modelMatrix[2];
    float3 lightDir;
};

[shader("closesthit")]
void main(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const uint primIndex = PrimitiveIndex();
    const uint instanceId = InstanceID(); // Replaces gl_InstanceCustomIndexEXT

    // Precompute the per-instance offsets
    uint vertexOffset = 0;
    uint indexOffset = 0;
    if (instanceId > 0)
    {
        vertexOffset = 24; // 24 vertices for the grid
        indexOffset = 36;  // 36 indices for the grid
    }

    const uint index0 = indices[indexOffset + primIndex * 3 + 0];
    const uint index1 = indices[indexOffset + primIndex * 3 + 1];
    const uint index2 = indices[indexOffset + primIndex * 3 + 2];

    // Clean, direct native structured buffer indexing
    const Vertex v0 = vertices[index0 + vertexOffset];
    const Vertex v1 = vertices[index1 + vertexOffset];
    const Vertex v2 = vertices[index2 + vertexOffset];

    const float3 barycentrics = float3(1.0f - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    float3 normal = normalize(v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z);
	
    // Transform to world space (using reverse multiplication order for row-major matrix)
    normal = normalize(mul(float4(normal, 0.0f), modelMatrix[instanceId]).xyz);

    float minTerm = 0.05f;
    float3 baseColor = float3(1.0f, 1.0f, 1.0f);
    float3 lDir = normalize(-lightDir);
    float NdotL = dot(normal, lDir);

    if (NdotL > 0.0f)
    {
        // Declare local instance of the secondary shadow payload structure
        ShadowPayload shadowPayload;
        shadowPayload.inShadow = true; // Assume shadowed until a miss shader hits

        // Compute world-space hit position using DXR system variables
        float3 worldPos = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
        float3 rayOrigin = worldPos + normal * 0.001f; // Offset to avoid surface acne self-intersection
        float3 rayDirection = lDir;

        RayDesc ray;
        ray.Origin    = rayOrigin;
        ray.TMin      = 0.001f;
        ray.Direction = rayDirection;
        ray.TMax      = 10000.0f;

        // D3D12 combined shadow ray optimization flags
        uint rayFlags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;

        // Trace nested shadow ray
        TraceRay(
            topLevelAS,
            rayFlags,
            0xFF, // Cull mask
            0,    // SBT record offset for shadow rays (miss index 1)
            0,    // Multiplier contribution 
            1,    // Miss index 1 explicitly targets our shadow miss shader
            ray,
            shadowPayload
        );
		
        if (shadowPayload.inShadow)
        {
            baseColor *= minTerm; // Darken if in shadow
        }
        else
        {
            baseColor *= NdotL; // Otherwise normal diffuse lighting
        }
    }
    else
    {
        baseColor *= 0.01f; // Darken if on the backface side
    }

    payload.color = float4(baseColor, 1.0f);
}
)raw";
constexpr auto g_RCHitSize = std::char_traits<char>::length(g_pRCHitShader);


constexpr const char* g_pRMissShadowShader =
R"raw(
struct ShadowPayload {
    bool inShadow;
};

[shader("miss")]
void main(inout ShadowPayload payload)
{
    // The shadow ray missed all geometry, meaning this pixel position is lit!
    payload.inShadow = false;
}
)raw";
constexpr auto g_RMissShadowSize = std::char_traits<char>::length(g_pRMissShadowShader);


constexpr const char* g_pVSShader =
R"raw(
struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

VSOutput main(uint vertexID : SV_VertexID)
{
    // Local lookups matching your hardcoded GLSL arrays
    const float2 vertices[6] = 
    {
        float2(-1.0f,  1.0f),
        float2( 1.0f,  1.0f),
        float2( 1.0f, -1.0f),
        float2(-1.0f,  1.0f),
        float2( 1.0f, -1.0f),
        float2(-1.0f, -1.0f)
    };

    const float2 uvs[6] = 
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 1.0f)
    };

    VSOutput output;
    output.pos = float4(vertices[vertexID], 0.0f, 1.0f);
    output.uv = uvs[vertexID];
    return output;
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);

constexpr const char* g_pPSShader =
R"raw(
struct PSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

Texture2D samplerTexture : register(t0);
SamplerState samplerObj : register(s1);

float4 main(PSInput input) : SV_Target
{
    return samplerTexture.Sample(samplerObj, input.uv);
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#else
constexpr const char* g_pRGenShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

layout(binding = 0, rgba32f) uniform image2D outputImage;
layout(binding = 1) uniform accelerationStructureEXT topLevelAS;
layout(binding = 2) uniform RayData
{
	mat4 viewInv;
	mat4 projInv;
} rayData;

layout(location = 0) rayPayloadEXT vec4 payload;

void main()
{
	const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
	const vec2 inUV = pixelCenter/vec2(gl_LaunchSizeEXT.xy);
	vec2 d = inUV * 2.0 - 1.0;
	d.y = -d.y;

	vec4 origin = rayData.viewInv * vec4(0,0,0,1);
	vec4 target = rayData.projInv * vec4(d.x, d.y, 1, 1);
	vec4 direction = rayData.viewInv * vec4(normalize(target.xyz), 0);

	float tmin = 0.001;
	float tmax = 10000.0;

	payload = vec4(d, 0.0, 1.0);

    traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, origin.xyz, tmin, direction.xyz, tmax, 0);

    imageStore(outputImage, ivec2(gl_LaunchIDEXT.xy), payload);
}
)raw";
constexpr auto g_RGenSize = std::char_traits<char>::length(g_pRGenShader);


constexpr const char* g_pRMissShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec4 payload;

layout(push_constant) uniform PC
{
	vec3 color;
} pc;

vec3 verticalGradient(vec3 topColor, float uvY)
{
    return topColor * uvY;
}

void main()
{
	vec2 uv = payload.xy;
	vec3 color = verticalGradient(pc.color, 1.0 - uv.y);
    payload = vec4(color, 1.0);
}
)raw";
constexpr auto g_RMissSize = std::char_traits<char>::length(g_pRMissShader);


constexpr const char* g_pRCHitShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

struct ShadowPayload
{
	bool inShadow;
};

layout(location = 0) rayPayloadInEXT vec4 payload;
layout(location = 1) rayPayloadEXT ShadowPayload shadowPayload;

hitAttributeEXT vec2 attribs;

struct Vertex
{
    vec3 pos;
	vec2 uv;
    vec3 normal;
};

layout(set = 0, binding = 1) uniform accelerationStructureEXT topLevelAS;

layout(set = 1, binding = 0) readonly buffer VertexBuffer
{
    vec4 vertices[];
};

layout(set = 1, binding = 1) readonly buffer IndexBuffer
{
    uint indices[];
};

layout(set = 1, binding = 2) uniform HitData
{
	mat4 modelMatrix[2];
	vec3 lightDir;
};

Vertex GetVertex(uint index, uint vertexOffset)
{
    uint baseIndex = 2 * (index + vertexOffset);
    Vertex v;
    v.pos = vertices[baseIndex].xyz;
    v.uv = vec2(vertices[baseIndex].w, vertices[baseIndex + 1].x);
    v.normal = vertices[baseIndex + 1].yzw;
    return v;
}

uint GetIndex(uint primIndex, uint vertexInTri, uint indexOffset)
{
    return indices[indexOffset + primIndex * 3 + vertexInTri];
}

void main()
{
    const uint primIndex = gl_PrimitiveID;
    const uint instanceId = gl_InstanceCustomIndexEXT;

    // Precompute the per-instance offsets only once
    uint vertexOffset = 0;
    uint indexOffset = 0;
    if (instanceId > 0)
    {
        vertexOffset = 24; // 24 vertices for the grid
        indexOffset = 36;  // 36 indices for the grid
    }

    const uint index0 = GetIndex(primIndex, 0, indexOffset);
    const uint index1 = GetIndex(primIndex, 1, indexOffset);
    const uint index2 = GetIndex(primIndex, 2, indexOffset);

    const Vertex v0 = GetVertex(index0, vertexOffset);
    const Vertex v1 = GetVertex(index1, vertexOffset);
    const Vertex v2 = GetVertex(index2, vertexOffset);

	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	vec3 normal = normalize(v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z);
	
    normal = normalize((modelMatrix[instanceId] * vec4(normal, 0.0)).xyz); // Transform to world space

	float minTerm = 0.05;
    vec3 baseColor = vec3(1.0, 1.0, 1.0);
	vec3 lDir = normalize(-lightDir);
	float NdotL = dot(normal, lDir);
	if (NdotL > 0.0)
	{
		shadowPayload.inShadow = true; // Assume not shadowed

		// Compute world-space hit position
		vec3 worldPos = gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT;
		vec3 rayOrigin = worldPos + normal * 0.001; // Offset a little to avoid self-hit
		vec3 rayDirection = lDir;   // Towards light (negate because lightDir points TO surface)

		traceRayEXT(
			topLevelAS, // (you must declare this somewhere, usually a uniform acceleration structure)
			gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT,
			0xFF,       // cull mask
			0, 0, 1,    // SBT record offsets for shadow rays
			rayOrigin,
			0.001,      // min t
			rayDirection,
			10000.0,     // max t
			1           // payload location
		);
		
		if (shadowPayload.inShadow)
		{
			baseColor *= minTerm; // Darken if in shadow
		}
		else
		{
			baseColor *= NdotL; // Otherwise normal lighting
		}
	}
	else
	{
		baseColor *= 0.01; // Darken if in shadow
	}

    payload = vec4(baseColor, 1.0);
}
)raw";
constexpr auto g_RCHitSize = std::char_traits<char>::length(g_pRCHitShader);


constexpr const char* g_pRMissShadowShader =
R"raw(
#version 460
#extension GL_EXT_ray_tracing : require

struct ShadowPayload
{
	bool inShadow;
};

layout(location = 1) rayPayloadInEXT ShadowPayload payload;

void main()
{
	payload.inShadow = false;
}
)raw";
constexpr auto g_RMissShadowSize = std::char_traits<char>::length(g_pRMissShadowShader);


constexpr const char* g_pVSShader =
R"raw(
#version 450

layout (location = 0) out vec2 outUV;

vec2 vertices[] = 
{
	vec2(-1.0f,  1.0f),
	vec2( 1.0f,  1.0f),
	vec2( 1.0f, -1.0f),
	vec2(-1.0f,  1.0f),
	vec2( 1.0f, -1.0f),
	vec2(-1.0f, -1.0f)
};

vec2 uvs[] = 
{
	vec2(0.0f, 0.0f),
	vec2(1.0f, 0.0f),
	vec2(1.0f, 1.0f),
	vec2(0.0f, 0.0f),
	vec2(1.0f, 1.0f),
	vec2(0.0f, 1.0f)
};

void main()
{
	gl_Position = vec4(vertices[gl_VertexIndex], 0.0f, 1.0f);
	outUV = uvs[gl_VertexIndex];
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);

constexpr const char* g_pPSShader =
R"raw(
#version 450

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform texture2D samplerTexture;
layout (binding = 1) uniform sampler samplerObj;

void main()
{
	outColor = texture(sampler2D(samplerTexture, samplerObj), inUV);
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#endif
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
struct RayPayload
{
	float4 color;
};

[shader("closesthit")]
void main(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	float u = attribs.barycentrics.x;
	float v = attribs.barycentrics.y;
	float w = 1.0f - u - v;

	payload.color = float4(w, u, v, 1.0f);
}
)raw";
constexpr auto g_RCHitSize = std::char_traits<char>::length(g_pRCHitShader);


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
	vec4 target = rayData.projInv * vec4(d.x, d.y, 1, 1) ;
	vec4 direction = rayData.viewInv*vec4(normalize(target.xyz), 0) ;

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

layout(location = 0) rayPayloadInEXT vec4 payload;

hitAttributeEXT vec2 attribs;

void main()
{
    float u = attribs.x;
    float v = attribs.y;
    float w = 1.0 - u - v;

    payload = vec4(w, u, v, 1.0);
}
)raw";
constexpr auto g_RCHitSize = std::char_traits<char>::length(g_pRCHitShader);


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
#pragma once


#include <string_view>


#if defined(USE_D3D12)
constexpr const char* g_pRGenShader =
R"raw(
RWTexture2D<float4> outputImage : register(u0);
RaytracingAccelerationStructure topLevelAS : register(t1);

struct RayPayload
{
    float4 color;
};

[shader("raygeneration")]
void main()
{
    const uint2 launchID = DispatchRaysIndex().xy;
    const uint2 launchSize = DispatchRaysDimensions().xy;

    float2 uv = float2(launchID) / float2(launchSize);
    
    RayPayload payload;
    payload.color = float4(uv, 0.0f, 1.0f); // simple gradient initialization

    RayDesc ray;
    ray.Origin    = float3(0.0f, 0.0f, 0.0f);
    ray.TMin      = 0.0f;
    ray.Direction = float3(0.0f, 0.0f, 1.0f);
    ray.TMax      = 10000.0f;

    // TraceRay replaces traceRayEXT
    // Arguments: AS, RayFlags, InstanceMask, ShadingRateIndexOffset, MultiplierForGeometryContribution, MissShaderIndex, Ray, Payload
    TraceRay(
        topLevelAS,
        RAY_FLAG_NONE,
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

cbuffer PC : register(b2)
{
    float3 pcColor;
};

[shader("miss")]
void main(inout RayPayload payload)
{
    payload.color = float4(pcColor, 1.0f);
}
)raw";
constexpr auto g_RMissSize = std::char_traits<char>::length(g_pRMissShader);


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

layout(set = 0, binding = 0, rgba32f) uniform image2D outputImage;
layout(set = 0, binding = 1) uniform accelerationStructureEXT topLevelAS;

layout(location = 0) rayPayloadEXT vec4 payload;

void main()
{
    const uvec2 launchID = gl_LaunchIDEXT.xy;
    const uvec2 launchSize = gl_LaunchSizeEXT.xy;

    vec2 uv = vec2(launchID) / vec2(launchSize);
    payload = vec4(uv, 0.0, 1.0); // simple gradient
    traceRayEXT(
        topLevelAS,                     // acceleration structure
        gl_RayFlagsNoneEXT,            // ray flags
        0xFF,                           // cull mask
        0,                              // sbt record offset
        0,                              // sbt record stride
        0,                              // miss index
        vec3(0.0, 0.0, 0.0),            // origin
        0.0,                            // min t
        vec3(0.0, 0.0, 1.0),            // direction
        10000.0,                        // max t
        0                               // payload location
    );

    imageStore(outputImage, ivec2(launchID), payload);
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

void main()
{
    payload = vec4(pc.color, 1.0);
}
)raw";
constexpr auto g_RMissSize = std::char_traits<char>::length(g_pRMissShader);


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
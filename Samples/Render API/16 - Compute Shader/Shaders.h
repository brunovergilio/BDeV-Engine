#pragma once


#include <string_view>


#if defined(USE_D3D12)
constexpr const char* g_pVSShader =
R"raw(
struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

static const float2 vertices[] = 
{
	float2(-1.0f,  1.0f),
	float2( 1.0f,  1.0f),
	float2( 1.0f, -1.0f),
	float2(-1.0f,  1.0f),
	float2( 1.0f, -1.0f),
	float2(-1.0f, -1.0f)
};

static const float2 uvs[] = 
{
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(1.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 1.0f),
	float2(0.0f, 1.0f)
};

VSOutput main(uint id : SV_VertexId)
{
	VSOutput output;
	output.pos = float4(vertices[id], 0.0f, 1.0f);
	output.uv = uvs[id];
	return output;
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);

constexpr const char* g_pPSShader =
R"raw(
struct VSOutput
{
    float4 position     : SV_Position;
    float2 outTexCoords : TEXCOORD0;
};

Texture2D samplerTexture : register(t0);
SamplerState samplerObj  : register(s1);

float4 main(VSOutput input) : SV_Target
{
    return samplerTexture.Sample(samplerObj, input.outTexCoords);
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);

constexpr const char* g_pCSShader =
R"raw(
RWTexture2D<float4> outImage : register(u0);

cbuffer PushConstants : register(b1)
{
	float3 pc;
};

[numthreads(8, 8, 1)]
void main(uint3 gid : SV_DispatchThreadID)
{
	int2 pixel = int2(gid.xy);
	if (pixel.x >= int(pc.x) || pixel.y >= int(pc.y))
	{
		return;
	}

	float2 uv = pixel / pc.xy;
	float3 color = 0.5 + 0.5 * cos(pc.z + uv.xyx * 6.2831 + float3(0, 2, 4));

	
	outImage[pixel] = float4(color, 1.0);
}
)raw";
constexpr auto g_CSSize = std::char_traits<char>::length(g_pCSShader);
#else
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

constexpr const char* g_pCSShader =
R"raw(
#version 450

layout (local_size_x = 8, local_size_y = 8) in;

layout (rgba8, binding = 0) uniform writeonly image2D outImage;
layout (push_constant) uniform PushConstants
{
    vec2 resolution;
    float time;
} pc;

void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    if (pixel.x >= int(pc.resolution.x) || pixel.y >= int(pc.resolution.y))
	{
		return;
	}

    vec2 uv = pixel / pc.resolution;
    vec3 color = 0.5 + 0.5 * cos(pc.time + uv.xyx * 6.2831 + vec3(0, 2, 4));

    imageStore(outImage, pixel, vec4(color, 1.0));
}
)raw";
constexpr auto g_CSSize = std::char_traits<char>::length(g_pCSShader);
#endif
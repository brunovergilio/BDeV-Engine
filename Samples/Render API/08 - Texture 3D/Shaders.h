#pragma once


#include <string_view>


#if defined(USE_D3D12)
constexpr const char* g_pVSShader =
R"raw(
struct VSInput
{
    float3 inPos       : POSITION;
    float2 inTexCoords : TEXCOORD0;
};

struct VSOutput
{
    float2 outTexCoords : TEXCOORD0;
    float4 position     : SV_Position;
};

cbuffer UBO : register(b0)
{
    matrix wvp; 
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.outTexCoords = input.inTexCoords;
    output.position = mul(wvp, float4(input.inPos, 1.0f));
    return output;
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);


constexpr const char* g_pPSShader =
R"raw(
struct VSOutput
{
    float2 outTexCoords : TEXCOORD0;
    float4 position     : SV_Position;
};

// layout (binding = 1) uniform texture2DArray samplerTexture;
Texture3D samplerTexture : register(t1);

// layout (binding = 2) uniform sampler samplerObj;
SamplerState samplerObj       : register(s2);

// layout (push_constant) uniform PushConstants
cbuffer PushConstants : register(b3) // Choose an available b-register slot
{
    float depth;
};

float4 main(VSOutput input) : SV_Target
{
    // Texture2DArray sampling requires a float3 coordinate: (u, v, array_slice)
    float3 uvw = float3(input.outTexCoords, depth);
    return samplerTexture.Sample(samplerObj, uvw);
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#else
constexpr const char* g_pVSShader =
R"raw(
#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoords;

layout (location = 0) out vec2 outTexCoords;

layout (binding = 0) uniform UBO 
{
	mat4 wvp;
} ubo;

void main() 
{
	outTexCoords = inTexCoords;
	gl_Position = ubo.wvp * vec4(inPos.xyz, 1.0);
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);


constexpr const char* g_pPSShader =
R"raw(
#version 450

layout (location = 0) in vec2 inTexCoords;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform texture3D samplerTexture;
layout (binding = 2) uniform sampler samplerObj;

layout (push_constant) uniform PushConstants
{
	float depth;
} pushConstants;

void main()
{
	outColor = texture(sampler3D(samplerTexture, samplerObj), vec3(inTexCoords, pushConstants.depth));
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#endif
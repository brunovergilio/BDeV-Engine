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

// layout (binding = 1) uniform texture2D samplerTexture;
Texture2D samplerTexture : register(t1);

// layout (binding = 2) uniform sampler samplerObj;
SamplerState samplerObj  : register(s2);

float4 main(VSOutput input) : SV_Target
{
    // texture(sampler2D(...), ...) becomes Sample(...)
    return samplerTexture.Sample(samplerObj, input.outTexCoords);
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

layout (binding = 1) uniform texture2D samplerTexture;
layout (binding = 2) uniform sampler samplerObj;

void main()
{
	outColor = texture(sampler2D(samplerTexture, samplerObj), inTexCoords);
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#endif
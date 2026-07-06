#pragma once


#include <string_view>

#if defined(USE_D3D12)
constexpr const char* g_pVSShader =
R"raw(
struct VSInput
{
    float3 inPos    : POSITION;
    float4 inColor  : COLOR;
};

struct VSOutput
{
    float4 outColor : COLOR;
    float4 position : SV_Position;
};

cbuffer UBO : register(b0)
{
    matrix wvp; 
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.outColor = input.inColor;
    output.position = mul(wvp, float4(input.inPos, 1.0f));
    return output;
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);


constexpr const char* g_pPSShader =
R"raw(
struct VSOutput
{
    float4 outColor : COLOR;
    float4 position : SV_Position;
};

float4 main(VSOutput input) : SV_Target
{
    return input.outColor;
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#else
constexpr const char* g_pVSShader =
R"raw(
#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform UBO 
{
	mat4 wvp;
} ubo;

void main() 
{
	outColor = inColor;
	gl_Position = ubo.wvp * vec4(inPos.xyz, 1.0);
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);


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
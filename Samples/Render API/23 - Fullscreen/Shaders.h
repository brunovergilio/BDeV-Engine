#pragma once


#include <string_view>


#if defined(USE_D3D12)
constexpr const char* g_pVSShader =
R"raw(
static const float2 vertices[] = 
{
	float2(-0.7f, -0.7f),
	float2( 0.0f,  0.7f),
	float2( 0.7f, -0.7f)
};

float4 main(uint vid : SV_VertexID) : SV_POSITION
{
	return float4(vertices[vid], 0.0f, 1.0f);
}
)raw";


constexpr const char* g_pPSShader =
R"raw(
float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
)raw";
#else
constexpr const char* g_pVSShader =
R"raw(
#version 450

vec2 vertices[] = 
{
	vec2(-0.7f, -0.7f),
	vec2( 0.0f,  0.7f),
	vec2( 0.7f, -0.7f)
};

void main()
{
	gl_Position = vec4(vertices[gl_VertexIndex], 0.0f, 1.0f);
}
)raw";


constexpr const char* g_pPSShader =
R"raw(
#version 450

layout (location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
)raw";
#endif


constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
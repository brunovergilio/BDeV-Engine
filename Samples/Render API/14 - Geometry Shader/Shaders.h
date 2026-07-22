#pragma once


#include <string_view>


#if defined(USE_D3D12)
constexpr const char* g_pVSShader =
R"raw(
struct VSInput
{
	float3 inPos : POSITION;
	float4 inColor : COLOR;
};

struct VSOutput
{
	float4 outPos : SV_POSITION;
	float4 outColor : COLOR;
};

cbuffer UBO : register(b0)
{
	matrix wvp;
};

VSOutput main(VSInput input)
{
	VSOutput output;
	output.outPos = mul(wvp, float4(input.inPos, 1.0));
	output.outColor = input.inColor;

	return output;
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);


constexpr const char* g_pPSShader =
R"raw(
struct PSInput
{
	float4 inPos : SV_POSITION;
	float4 inColor : COLOR;
};

float4 main(PSInput input) : SV_TARGET
{
	return input.inColor;
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);


constexpr const char* g_pVSDebugShader =
R"raw(
struct VSInput
{
	float3 inPos : POSITION;
	float3 inNormal : NORMAL;
};

struct VSOutput
{
	float4 outPos : SV_POSITION;
	float3 outNormal : NORMAL;
};

VSOutput main(VSInput input)
{
	VSOutput output;
	output.outPos = float4(input.inPos, 1.0);
	output.outNormal = input.inNormal;

	return output;
}
)raw";
constexpr auto g_VSDebugSize = std::char_traits<char>::length(g_pVSDebugShader);


constexpr const char* g_pGSDebugShader =
R"raw(
struct GSInput
{
	float4 inPos : SV_POSITION;
	float3 inNormal : NORMAL;
};

struct GSOutput
{
	float4 outPos : SV_POSITION;
	float4 outColor : COLOR;
};

cbuffer UBO : register(b0)
{
	matrix w;
	matrix vp;
};

[maxvertexcount(6)]
void main(triangle GSInput input[3], inout LineStream<GSOutput> outputStream)
{
	GSOutput output;
	float normalLength = 0.2;
	for (int i = 0; i < 3; ++i)
	{
		float3 pos = input[i].inPos.xyz;
		float3 normal = input[i].inNormal;

		output.outPos = mul(vp, mul(w, float4(pos, 1.0f)));
		output.outColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
		outputStream.Append(output);

		output.outPos = mul(vp, mul(w, float4(pos + normal * normalLength, 1.0f)));
		output.outColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
		outputStream.Append(output);

		outputStream.RestartStrip();
	}
}
)raw";
constexpr auto g_GSDebugSize = std::char_traits<char>::length(g_pGSDebugShader);
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


constexpr const char* g_pVSDebugShader =
R"raw(
#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

layout (location = 0) out vec3 outNormal;

void main() 
{
	outNormal = inNormal;
	gl_Position = vec4(inPos.xyz, 1.0);
}
)raw";
constexpr auto g_VSDebugSize = std::char_traits<char>::length(g_pVSDebugShader);


constexpr const char* g_pGSDebugShader =
R"raw(
#version 450

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

layout (binding = 0) uniform UBO 
{
	mat4 w;
	mat4 vp;
} ubo;

layout (location = 0) in vec3 inNormal[];

layout (location = 0) out vec4 outColor;

void main()
{	
	float normalLength = 0.2;
	for (int i = 0; i < gl_in.length(); ++i)
	{
		vec3 pos = gl_in[i].gl_Position.xyz;
		vec3 normal = inNormal[i].xyz;

		gl_Position = ubo.vp * (ubo.w * vec4(pos, 1.0));
		outColor = vec4(1.0, 0.0, 0.0, 1.0);
		EmitVertex();

		gl_Position = ubo.vp * (ubo.w * vec4(pos + normal * normalLength, 1.0));
		outColor = vec4(0.0, 0.0, 1.0, 1.0);
		EmitVertex();

		EndPrimitive();
	}
}
)raw";
constexpr auto g_GSDebugSize = std::char_traits<char>::length(g_pGSDebugShader);
#endif
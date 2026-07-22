#pragma once


#include <string_view>


#if defined(USE_D3D12)
constexpr const char* g_pVSShaderOffscreen =
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
    output.outColor = input.inColor;
    output.outPos = mul(wvp, float4(input.inPos, 1.0f));
    return output;
}
)raw";
constexpr auto g_VSSizeOffscreen = std::char_traits<char>::length(g_pVSShaderOffscreen);

constexpr const char* g_pPSShaderOffscreen =
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
constexpr auto g_PSSizeOffscreen = std::char_traits<char>::length(g_pPSShaderOffscreen);

constexpr const char* g_pVSShader =
R"raw(
cbuffer PushConstants : register(b3)
{
	float4 posUV[6];
};

struct VSOutput
{
	float4 outPos : SV_POSITION;
	float2 outTexCoords : TEXCOORD0;
};

VSOutput main(uint vid : SV_VertexID)
{
	VSOutput output;
	output.outPos = float4(posUV[vid].xy, 0.0f, 1.0f);
	output.outTexCoords = float2(posUV[vid].zw);
	return output;
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);

constexpr const char* g_pPSShader =
R"raw(
struct PSInput
{
	float4 inPos : SV_POSITION;
	float2 inTexCoords : TEXCOORD0;
};

SamplerState samplerObj : register(s0);
Texture2D colorTexture : register(t1);
Texture2D depthTexture : register(t2);

cbuffer PushConstants2 : register(b4)
{
	float2 screenSize;
};

float4 main(PSInput input) : SV_TARGET
{
	float4 finalColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	// branching - ugly, but simple
	if (input.inPos.x < screenSize.x * screenSize.y)
	{
		float d = 1.0f - depthTexture.Sample(samplerObj, input.inTexCoords).r;
		finalColor = float4(d, d, d, 1.0f);
	}
	else
	{
		finalColor = colorTexture.Sample(samplerObj, input.inTexCoords);
	}
	return finalColor;
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#else
constexpr const char* g_pVSShaderOffscreen =
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
constexpr auto g_VSSizeOffscreen = std::char_traits<char>::length(g_pVSShaderOffscreen);


constexpr const char* g_pPSShaderOffscreen =
R"raw(
#version 450

layout (location = 0) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main()
{
	outColor = inColor;
}
)raw";
constexpr auto g_PSSizeOffscreen = std::char_traits<char>::length(g_pPSShaderOffscreen);


constexpr const char* g_pVSShader =
R"raw(
#version 450

layout (location = 0) out vec2 outTexCoords;

layout (push_constant) uniform PushConstants
{
	vec4 posUV[6];
} pushConstants;

void main() 
{
	outTexCoords = vec2(pushConstants.posUV[gl_VertexIndex].zw);
	gl_Position = vec4(pushConstants.posUV[gl_VertexIndex].xy, 0.0, 1.0);
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);


constexpr const char* g_pPSShader =
R"raw(
#version 450

layout (location = 0) in vec2 inTexCoords;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler samplerObj;
layout (binding = 1) uniform texture2D colorTexture;
layout (binding = 2) uniform texture2D depthTexture;

layout (push_constant) uniform PushConstants2
{
	layout(offset = 96) vec2 screenSize;
} pushConstants2;

void main()
{
	vec4 finalColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	if (gl_FragCoord.x < pushConstants2.screenSize.x * pushConstants2.screenSize.y)
	{
		float d = 1.0f - texture(sampler2D(depthTexture, samplerObj), inTexCoords).r;
		finalColor = vec4(d, d, d, 1.0f);
	}
	else
	{
		finalColor = texture(sampler2D(colorTexture, samplerObj), inTexCoords);
	}
	outColor = finalColor;
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#endif
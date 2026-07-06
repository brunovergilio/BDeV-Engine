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

Texture2D samplerTexture : register(t1);
SamplerState samplerObj  : register(s2);

float4 main(VSOutput input) : SV_Target
{
    return samplerTexture.Sample(samplerObj, input.outTexCoords);
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);


constexpr const char* g_pVSSkyBoxShader =
R"raw(
struct SkyBoxVSInput
{
    float3 inPos : POSITION;
};

struct SkyBoxVSOutput
{
    float3 outTexCoords : TEXCOORD0;
    float4 position     : SV_Position;
};

cbuffer UBO : register(b0)
{
    matrix wvp; 
};

SkyBoxVSOutput main(SkyBoxVSInput input)
{
    SkyBoxVSOutput output;
    output.outTexCoords = input.inPos;
    
    // Transform coordinates and perform the skybox depth trick (.xyww)
    float4 clipPos = mul(wvp, float4(input.inPos, 1.0f));
    output.position = clipPos.xyww;
    
    return output;
}
)raw";
constexpr auto g_VSSkyBoxSize = std::char_traits<char>::length(g_pVSSkyBoxShader);


constexpr const char* g_pPSSkyBoxShader =
R"raw(
struct SkyBoxVSOutput
{
    float3 outTexCoords : TEXCOORD0;
    float4 position     : SV_Position;
};

// layout (binding = 1) uniform textureCube samplerTexture;
TextureCube samplerTexture : register(t1);

// layout (binding = 2) uniform sampler samplerObj;
SamplerState samplerObj    : register(s2);

float4 main(SkyBoxVSOutput input) : SV_Target
{
    // TextureCube sampling requires a float3 direction vector
    return samplerTexture.Sample(samplerObj, input.outTexCoords);
}
)raw";
constexpr auto g_PSSkyBoxSize = std::char_traits<char>::length(g_pPSSkyBoxShader);
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


constexpr const char* g_pVSSkyBoxShader =
R"raw(
#version 450

layout (location = 0) in vec3 inPos;

layout (location = 0) out vec3 outTexCoords;

layout (binding = 0) uniform UBO
{
	mat4 wvp;
} ubo;

void main() 
{
	gl_Position = (ubo.wvp * vec4(inPos.xyz, 1.0)).xyww;
	outTexCoords = inPos;
}
)raw";
constexpr auto g_VSSkyBoxSize = std::char_traits<char>::length(g_pVSSkyBoxShader);


constexpr const char* g_pPSSkyBoxShader =
R"raw(
#version 450

layout (location = 0) in vec3 inTexCoords;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform textureCube samplerTexture;
layout (binding = 2) uniform sampler samplerObj;

void main()
{
	outColor = texture(samplerCube(samplerTexture, samplerObj), inTexCoords);
}
)raw";
constexpr auto g_PSSkyBoxSize = std::char_traits<char>::length(g_pPSSkyBoxShader);
#endif
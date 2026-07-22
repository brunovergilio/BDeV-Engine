#pragma once


#include <string_view>


#if defined(USE_D3D12)
constexpr const char* g_pVSShader =
R"raw(
struct VSOutput
{
    float4 pos : SV_Position;
};

VSOutput main()
{
    VSOutput output;
    output.pos = float4(0.0f, 0.0f, 0.0f, 1.0f);
    return output;
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);

constexpr const char* g_pHSShader =
R"raw(
cbuffer PC : register(b0)
{
	float4 tessData0;
	float2 tessData1;
};

struct VSOutput
{
	float4 pos : SV_POSITION;
};

struct HSOutput
{
	float4 pos : POSITION;
};

struct HSConstantOutput
{
    float outer[4] : SV_TessFactor;
    float inner[2] : SV_InsideTessFactor;
};

HSConstantOutput PatchConstantFunc(InputPatch<VSOutput, 1> ip)
{
    HSConstantOutput output;
    
    output.outer[0] = tessData0.x;
    output.outer[1] = tessData0.y;
    output.outer[2] = tessData0.z;
	output.outer[3] = tessData0.w;
	output.inner[0] = tessData1.x;
	output.inner[1] = tessData1.y;
    
    return output;
}

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(1)]
[patchconstantfunc("PatchConstantFunc")]
HSOutput main(InputPatch<VSOutput, 1> ip, uint i : SV_OutputControlPointID)
{
	HSOutput output;
	output.pos = float4(0.0f, 0.0f, 0.0f, 1.0f);
	return output;
}
)raw";
constexpr auto g_HSSize = std::char_traits<char>::length(g_pHSShader);

constexpr const char* g_pHSTriShader =
R"raw(
cbuffer PC : register(b0)
{
	float4 tessData0;
	float2 tessData1;
};

struct VSOutput
{
	float4 pos : SV_POSITION;
};

struct HSOutput
{
	float4 pos : POSITION;
};

struct HSConstantOutput
{
    float outer[3] : SV_TessFactor;
    float inner : SV_InsideTessFactor;
};

HSConstantOutput PatchConstantFunc(InputPatch<VSOutput, 1> ip)
{
    HSConstantOutput output;
    
    output.outer[0] = tessData0.x;
    output.outer[1] = tessData0.y;
    output.outer[2] = tessData0.z;
	output.inner = tessData1.x;
    
    return output;
}

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(1)]
[patchconstantfunc("PatchConstantFunc")]
HSOutput main(InputPatch<VSOutput, 1> ip, uint i : SV_OutputControlPointID)
{
	HSOutput output;
	output.pos = float4(0.0f, 0.0f, 0.0f, 1.0f);
	return output;
}
)raw";
constexpr auto g_HSTriSize = std::char_traits<char>::length(g_pHSTriShader);

constexpr const char* g_pDSShader =
R"raw(
struct HSConstantOutput
{
	float outer[4] : SV_TessFactor;
	float inner[2] : SV_InsideTessFactor;
};

struct HSOutput
{
	float4 pos : POSITION;
};

struct DSOutput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

[domain("quad")]
DSOutput main(HSConstantOutput input, float2 uv : SV_DomainLocation, const OutputPatch<HSOutput, 1> patch)
{
	DSOutput output;
	float u = uv.x;
	float v = uv.y;

	// Fullscreen quad [-1, 1] in clip space
	output.pos = float4(u * 2.0f - 1.0f, v * 2.0f - 1.0f, 0.0f, 1.0f);
	output.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
	return output;
}
)raw";
constexpr auto g_DSSize = std::char_traits<char>::length(g_pDSShader);

constexpr const char* g_pDSTriShader =
R"raw(
struct HSConstantOutput
{
    float outer[3] : SV_TessFactor;
    float inner : SV_InsideTessFactor;
};

struct HSOutput
{
	float4 pos : POSITION;
};

struct DSOutput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

[domain("tri")]
DSOutput main(HSConstantOutput input, float3 uvw : SV_DomainLocation, const OutputPatch<HSOutput, 1> patch)
{
	DSOutput output;
	// Barycentric coordinates: uvw = (u, v, w)
	float u = uvw.x;
	float v = uvw.y;
	float w = uvw.z;

	// Barycentric - polar mapping trick
	float angle = v * 2.0f * 3.14159265f;
	float radius = u;

	// Convert polar to Cartesian
	float x = cos(angle) * radius;
	float y = sin(angle) * radius;

	output.pos = float4(x, y, 0.0f, 1.0f);
	output.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
	return output;
}
)raw";
constexpr auto g_DSTriSize = std::char_traits<char>::length(g_pDSTriShader);

constexpr const char* g_pPSShader =
R"raw(
struct PSInput
{
	float4 pos : SV_POSITION;
	float4 inColor : COLOR;
};

float4 main(PSInput input) : SV_Target
{
	return input.inColor;
}
)raw";
constexpr auto g_PSSize = std::char_traits<char>::length(g_pPSShader);
#else
constexpr const char* g_pVSShader =
R"raw(
#version 450

void main()
{
}
)raw";
constexpr auto g_VSSize = std::char_traits<char>::length(g_pVSShader);

constexpr const char* g_pHSShader =
R"raw(
#version 450

layout(vertices = 1) out;

layout(push_constant) uniform PC
{
	float tessData[6];
} pc;

void main()
{
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = pc.tessData[0]; // horizontal subdivisions
        gl_TessLevelOuter[1] = pc.tessData[1]; // vertical subdivisions
        gl_TessLevelOuter[2] = pc.tessData[2];
        gl_TessLevelOuter[3] = pc.tessData[3];
        gl_TessLevelInner[0] = pc.tessData[4];
        gl_TessLevelInner[1] = pc.tessData[5];
    }
}
)raw";
constexpr auto g_HSSize = std::char_traits<char>::length(g_pHSShader);

constexpr const char* g_pHSTriShader =
R"raw(
#version 450

layout(vertices = 1) out;

layout(push_constant) uniform PC
{
	float tessData[6];
} pc;

void main()
{
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = pc.tessData[0]; // horizontal subdivisions
        gl_TessLevelOuter[1] = pc.tessData[1]; // vertical subdivisions
        gl_TessLevelOuter[2] = pc.tessData[2];
        gl_TessLevelOuter[3] = pc.tessData[3];
        gl_TessLevelInner[0] = pc.tessData[4];
        gl_TessLevelInner[1] = pc.tessData[5];
    }
}
)raw";
constexpr auto g_HSTriSize = std::char_traits<char>::length(g_pHSTriShader);

constexpr const char* g_pDSShader =
R"raw(
#version 450

layout(quads, fractional_even_spacing, cw) in;

layout (location = 0) out vec4 outColor;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Fullscreen quad [-1, 1] in clip space
    gl_Position = vec4(u * 2.0 - 1.0, v * 2.0 - 1.0, 0.0, 1.0);
	outColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)raw";
constexpr auto g_DSSize = std::char_traits<char>::length(g_pDSShader);

constexpr const char* g_pDSTriShader =
R"raw(
#version 450

layout(triangles, fractional_even_spacing, cw) in;

layout (location = 0) out vec4 outColor;

void main()
{
    // Barycentric coordinates: gl_TessCoord = (u, v, w)
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float w = gl_TessCoord.z;

    // Barycentric - polar mapping trick
    float angle = v * 2.0 * 3.14159265;
    float radius = u;

    // Convert polar to Cartesian
    float x = cos(angle) * radius;
    float y = sin(angle) * radius;

    gl_Position = vec4(x, y, 0.0, 1.0);
	outColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)raw";
constexpr auto g_DSTriSize = std::char_traits<char>::length(g_pDSTriShader);

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
#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"


namespace PrimTexture
{
	struct Vertex
	{
		Float2 m_Pos;
		Float2 m_UV;
		Float4 m_Color;
	};

	struct Constants
	{
		Float4 m_Dims; // (xy - pos1, zw - pos2) all in screen sizes
		Float2 m_UVs[2];
		Float2 m_PixelSize; // (1.0 / screen size)
		Float2 m_Pad;
	};

#if defined(USE_D3D12)
#else
	constexpr const char* g_pVSLineShaderTexture =
		R"raw(
	#version 450

	layout (location = 0) out vec2 outUV;

	layout (push_constant) uniform Constants
	{
		vec4 dims; // (xy - pos1, zw - pos2) all in screen sizes
		vec2 uvs[2];
		vec2 pixelSize; // (1.0 / screen size)
	};

	void main()
	{
		vec2 pos[] =
		{
			dims.xy,
			dims.zw
		};

		vec2 vertOffset = pos[gl_VertexIndex];
		vec2 pixelPos = vertOffset + 0.5f;
		vec2 ndc = (pixelPos * pixelSize) * 2.0f - 1.0f;
		ndc.y = -ndc.y;

		outUV = uvs[gl_VertexIndex];
		gl_Position = vec4(ndc, 0.0f, 1.0f);
	}
	)raw";

	constexpr const char* g_pVSSquareShaderTexture =
	R"raw(
	#version 450

	layout (location = 0) out vec2 outUV;

	layout (push_constant) uniform Constants
	{
		vec4 dims; // (xy - pos1, zw - pos2) all in screen sizes
		vec2 uvs[2];
		vec2 pixelSize; // (1.0 / screen size)
		vec2 pad;
	};

	void main()
	{
		vec2 pos = dims.xy;
		vec2 size = dims.zw;
		vec2 vertOffset = vec2(float((gl_VertexIndex >> 1) & 1), float(gl_VertexIndex & 1));
		vec2 pixelPos = pos + vertOffset * size + 0.5f;
		vec2 ndc = (pixelPos * pixelSize) * 2.0f - 1.0f;
		ndc.y = -ndc.y;

		outUV = mix(uvs[0], uvs[1], vertOffset);
		gl_Position = vec4(ndc, 0.0f, 1.0f);
	}
	)raw";


	constexpr const char* g_pPSShaderTexture =
	R"raw(
	#version 450

	layout (push_constant) uniform ColorMultiplier
	{
		layout(offset = 48) vec4 colorMul;
	};

	layout (location = 0) in vec2 inTexCoords;

	layout (location = 0) out vec4 outColor;

	layout (binding = 1) uniform texture2D samplerTexture;
	layout (binding = 2) uniform sampler samplerObj;

	void main()
	{
		outColor = texture(sampler2D(samplerTexture, samplerObj), inTexCoords) * colorMul;
	}
	)raw";
#endif


	constexpr auto g_VSLineShaderTextureSize = std::char_traits<char>::length(g_pVSLineShaderTexture);
	constexpr auto g_VSSquaredShaderTextureSize = std::char_traits<char>::length(g_pVSSquareShaderTexture);
	constexpr auto g_PSShaderTextureSize = std::char_traits<char>::length(g_pPSShaderTexture);
}
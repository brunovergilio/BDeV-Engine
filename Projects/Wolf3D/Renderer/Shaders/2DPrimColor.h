#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"


namespace PrimColor
{
	struct Constants
	{
		Float4 m_Color;
		Float4 m_Dims; // (xy - pos1, zw - pos2) all in screen sizes
		Float2 m_PixelSize; // (1.0 / screen size)
	};

	#if defined(USE_D3D12)
	constexpr const char* g_pVSSquareShaderColor =
	R"raw(

	float4 main(uint vid : SV_VertexID) : SV_POSITION
	{
		return float4(vertices[vid], 0.0f, 1.0f);
	}
	)raw";


	constexpr const char* g_pPSShaderColor =
	R"raw(
	cbuffer Color : register(b0)
	{
		float4 g_color;
	};

	float4 main(float4 pos : SV_POSITION) : SV_TARGET
	{
		return g_color;
	}
	)raw";
	#else
	constexpr const char* g_pVSLineShaderColor =
	R"raw(
	#version 450

	layout (location = 0) out vec4 outColor;

	layout (push_constant) uniform Constants
	{
		vec4 color;
		vec4 dims; // (xy - pos1, zw - pos2) all in screen sizes
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

		outColor = color;
		gl_Position = vec4(ndc, 0.0f, 1.0f);
	}
	)raw";

	constexpr const char* g_pVSSquareShaderColor =
	R"raw(
	#version 450

	layout (location = 0) out vec4 outColor;

	layout (push_constant) uniform Constants
	{
		vec4 color;
		vec4 dims; // (xy - pos, zw - size) all in screen sizes
		vec2 pixelSize; // (1.0 / screen size)
	};

	void main()
	{
		vec2 pos = dims.xy;
		vec2 size = dims.zw;
		vec2 vertOffset = vec2(float((gl_VertexIndex >> 1) & 1), float(gl_VertexIndex & 1));
		vec2 pixelPos = pos + vertOffset * size + 0.5f;
		vec2 ndc = (pixelPos * pixelSize) * 2.0f - 1.0f;
		ndc.y = -ndc.y;

		outColor = color;
		gl_Position = vec4(ndc, 0.0f, 1.0f);
	}
	)raw";


	constexpr const char* g_pPSShaderColor =
	R"raw(
	#version 450

	layout (location = 0) in vec4 inColor;
	layout (location = 0) out vec4 outColor;

	void main()
	{
		outColor = inColor;
	}
	)raw";
	#endif


	constexpr auto g_VSLineShaderColorSize = std::char_traits<char>::length(g_pVSLineShaderColor);
	constexpr auto g_VSSquaredShaderColorSize = std::char_traits<char>::length(g_pVSSquareShaderColor);
	constexpr auto g_PSShaderColorSize = std::char_traits<char>::length(g_pPSShaderColor);
}
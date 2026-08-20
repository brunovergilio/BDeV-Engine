#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"


struct Constants
{
	Float4 m_Color;
	Float4 m_Dims; // (xy - pos, zw - size) all in screen sizes
	Float2 m_PixelSize; // (1.0 / screen size)
};


//#define USE_D3D12


#if defined(USE_D3D12)
constexpr const char* kpRenderLib = "BvRenderD3D12.dll";
constexpr const char* kpCompiler = "CreateDXCompiler";
constexpr ShaderLanguage kShaderLanguage = ShaderLanguage::kHLSL;
#else
constexpr const char* kpRenderLib = "BvRenderVk.dll";
constexpr const char* kpCompiler = "CreateSPIRVCompiler";
constexpr ShaderLanguage kShaderLanguage = ShaderLanguage::kGLSL;
#endif
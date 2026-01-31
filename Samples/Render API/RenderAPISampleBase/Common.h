#pragma once


#define USE_D3D12


#if defined(USE_D3D12)
constexpr const char* kpRenderLib = "BvRenderD3D12.dll";
constexpr const char* kpCompiler = "CreateDXCompiler";
constexpr ShaderLanguage kShaderLanguage = ShaderLanguage::kHLSL;
#else
constexpr const char* kpRenderLib = "BvRenderVk.dll";
constexpr const char* kpCompiler = "CreateSPIRVCompiler";
constexpr ShaderLanguage kShaderLanguage = ShaderLanguage::kGLSL;
#endif
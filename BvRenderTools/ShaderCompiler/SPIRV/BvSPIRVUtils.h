#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"


namespace BvSPIRVUtils
{
	bool CompileSPIRV(const ShaderSourceDesc& shaderDesc, BvVector<u8>& result, BvVector<u8>& errorMsg);
	bool ConvertSPIRVToHLSL(const BvVector<u8>& spirvBytes, BvVector<u8>& result, ShaderTarget shaderTarget);
}
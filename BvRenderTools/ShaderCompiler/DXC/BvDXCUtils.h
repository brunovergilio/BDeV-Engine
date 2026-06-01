#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/System/BvPlatformHeaders.h"
#include <wrl/client.h>
#include "Third Party/dxcapi.h"


namespace BvDXCUtils
{
	bool CompileToDXIL(IDxcUtils* pUtils, IDxcCompiler3* pCompiler, const ShaderSourceDesc& shaderDesc, BvVector<u8>& result, BvVector<u8>& errors);
}
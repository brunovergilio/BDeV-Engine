#pragma once


#include "BDeV/Core/RenderAPI/BvRenderCommon.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Utils/BvObject.h"


//BV_OBJECT_DEFINE_ID(IBvShader, "21c6bb95-306d-4e87-8c73-1ec34ddc383b");
//BV_OBJECT_ENABLE_ID_OPERATOR(IBvShader);
class IBvShader : public BvRCObj
{
	BV_NOCOPYMOVE(IBvShader);

public:
	virtual ShaderStage GetShaderStage() const = 0;
	virtual const BvVector<u8>& GetShaderBlob() const = 0;
	virtual const char* GetEntryPoint() const = 0;

protected:
	IBvShader() {}
	~IBvShader() {}
};
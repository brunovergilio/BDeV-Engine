#pragma once


#include "BvTexture.h"


BV_OBJECT_DEFINE_ID(IBvTextureView, "6178f332-f7bc-4465-bf96-a6713c733ece");
class IBvTextureView : public BvRCObj
{
	BV_NOCOPYMOVE(IBvTextureView);

public:
	virtual const TextureViewDesc& GetDesc() const = 0;

protected:
	IBvTextureView() {}
	~IBvTextureView() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvTextureView);
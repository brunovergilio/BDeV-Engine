#include "BvTextureLoaderCommon.h"


void BvTextureBlob::SelfDestroy()
{
	BV_DELETE(this);
}
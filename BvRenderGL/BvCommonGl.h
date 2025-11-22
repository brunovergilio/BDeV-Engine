#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BDeV/Core/RenderAPI/BvRenderCommon.h"

#if BV_PLATFORM_WIN32
#include <Windows.h>
#endif

#include <GL/glew.h>

#if BV_PLATFORM_WIN32
#include <GL/wglew.h>
#endif


#define BV_CREATE_CAST_TO_GL(Type) namespace Internal \
{ \
BV_INLINE Type##Gl* ToGl(I##Type* pObj) { return static_cast<Type##Gl*>(pObj); } \
BV_INLINE const Type##Gl* ToGl(const I##Type* pObj) { return static_cast<const Type##Gl*>(pObj); } \
}

#define TO_GL(pObj) Internal::ToGl(pObj)


class IBvResourceGl
{
public:
	virtual void Destroy() = 0;

protected:
	IBvResourceGl() {}
	virtual ~IBvResourceGl() {}
};


struct BvDeviceInfoGl
{
	static constexpr auto kMaxDeviceNameSize = 32u;
	static constexpr auto kMaxDriverNameSize = 64u;

	uint32_t majorVersion{};
	uint32_t minorVersion{};
	char driverName[kMaxDriverNameSize]{};
	char deviceName[kMaxDeviceNameSize]{};
	char vendorName[kMaxDeviceNameSize]{};
	char shaderVersionName[kMaxDeviceNameSize]{};

	struct
	{
		bool directStateAccess : 1;
		bool textureFilterAnisotropic : 1;
		bool polygonOffsetClamp : 1;
		bool nvConservativeRaster : 1;
		bool intelConservativeRaster : 1;
		bool depthBoundsTest : 1;
	} m_ExtendedFeatures{};
};


#define BV_GL_DEVICE_RES_DECL void SelfDestroy() override;
#define BV_GL_DEVICE_RES_DEF(Type) void Type::SelfDestroy() { m_pDevice->DestroyResource(this); }

#define BV_GL_IS_TYPE_VALID(uuid, Type) (uuid == BV_OBJECT_ID(I##Type) || uuid == BV_OBJECT_ID(Type##Gl))
#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"

// Platform-dependent stuff
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#define BV_VULKAN_DLL_NAME "vulkan-1.dll"
#else
	// Other platforms
#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)


// Defines for the Vulkan Memory Allocator so it uses functions loaded from Volk
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0


#include "Third Party/volk.h"
#include "Third Party/vk_mem_alloc.h"


// Have to re-include this again because the CreateSemaphore macro
// conflicts with the BvRenderEngine::CreateSemaphore function
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include "BDeV/Core/System/BvPlatformHeaders.h"
#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)


#define BV_CREATE_CAST_TO_VK(Type) namespace Internal \
{ \
BV_INLINE Type##Vk* ToVk(I##Type* pObj) { return static_cast<Type##Vk*>(pObj); } \
BV_INLINE const Type##Vk* ToVk(const I##Type* pObj) { return static_cast<const Type##Vk*>(pObj); } \
}

#define TO_VK(pObj) Internal::ToVk(pObj)


class IBvResourceVk
{
public:
	virtual void Destroy() = 0;

protected:
	IBvResourceVk() {}
	virtual ~IBvResourceVk() {}
};


#define BV_VK_DEVICE_RES_DECL void SelfDestroy() override;
#define BV_VK_DEVICE_RES_DEF(Type) void Type::SelfDestroy() { m_pDevice->DestroyResource(this); }
#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/Diagnostics/BvDiagnostics.h"


#include <dxgi.h>
#include <d3d12.h>
#include <wrl/client.h>
#include "d3dx12.h"
#include "d3d12video.h"


using Microsoft::WRL::ComPtr;


#if (BV_COMPILER == BV_COMPILER_MSVC)
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#endif


#include "Third Party/D3D12MemAlloc.h"


// Have to re-include this again because the CreateSemaphore macro
// conflicts with the BvRenderEngine::CreateSemaphore function
#if (BV_PLATFORM == BV_PLATFORM_WIN32)
#include "BDeV/Core/System/BvPlatformHeaders.h"
#endif // #if (BV_PLATFORM == BV_PLATFORM_WIN32)


#define BV_CREATE_CAST_TO_D3D12(Type) namespace Internal \
{ \
BV_INLINE Type##D3D12* ToD3D12(I##Type* pObj) { return static_cast<Type##D3D12*>(pObj); } \
BV_INLINE const Type##D3D12* ToD3D12(const I##Type* pObj) { return static_cast<const Type##D3D12*>(pObj); } \
}

#define TO_D3D12(pObj) Internal::ToD3D12(pObj)


class IBvResourceD3D12
{
public:
	virtual void Destroy() = 0;

protected:
	IBvResourceD3D12() {}
	virtual ~IBvResourceD3D12() {}
};


#define BV_D3D12_DEVICE_RES_DECL void SelfDestroy() override;
#define BV_D3D12_DEVICE_RES_DEF(Type) void Type::SelfDestroy() { m_pDevice->DestroyResource(this); }
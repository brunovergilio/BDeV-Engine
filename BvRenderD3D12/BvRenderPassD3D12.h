#pragma once


#include "BvCommonD3D12.h"
#include "BDeV/Core/RenderAPI/BvRenderPass.h"


class BvRenderDeviceD3D12;


class BvRenderPassD3D12 final : public IBvRenderPass, public IBvResourceD3D12
{
	BV_D3D12_DEVICE_RES_DECL;

public:
	BvRenderPassD3D12(BvRenderDeviceD3D12* pDevice, const RenderPassDesc& renderPassDesc);
	~BvRenderPassD3D12();

	BV_INLINE const RenderPassDesc& GetDesc() const override { return m_RenderPassDesc; }

	//BV_OBJECT_IMPL_INTERFACE(IBvRenderPassD3D12, IBvRenderPass, IBvRenderDeviceObject);

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	RenderPassDesc m_RenderPassDesc;
	RenderPassAttachment* m_pAttachments = nullptr;
	SubpassDependency* m_pDependencies = nullptr;
	SubpassDesc* m_pSubpasses = nullptr;
	AttachmentRef* m_pRefs = nullptr;
	ShadingRateAttachmentRef* m_pSRRefs = nullptr;
};


BV_CREATE_CAST_TO_D3D12(BvRenderPass)
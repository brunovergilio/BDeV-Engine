#pragma once


#include "BvCommonD3D12.h"
#include "BDeV/Core/RenderAPI/BvRenderPass.h"


class BvRenderDeviceD3D12;


class BvRenderPassD3D12 final : public IBvRenderPass, public IBvResourceD3D12
{
public:
	struct BarrierData
	{
		BvVector<std::pair<u32, u32>> m_SubpassBarrierIndices;

		struct StateData
		{
			u32 m_Index;
			D3D12_RESOURCE_STATES m_StateBefore;
			D3D12_RESOURCE_STATES m_StateAfter;
		};
		BvVector<StateData> m_States;
	};

	BvRenderPassD3D12(BvRenderDeviceD3D12* pDevice, const RenderPassDesc& renderPassDesc);
	~BvRenderPassD3D12();

	BV_INLINE const RenderPassDesc& GetDesc() const override { return m_RenderPassDesc; }
	BV_INLINE const auto GetBarrierData() const { return m_pBarrierData; }

private:
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	RenderPassDesc m_RenderPassDesc;
	RenderPassAttachment* m_pAttachments = nullptr;
	SubpassDependency* m_pDependencies = nullptr;
	SubpassDesc* m_pSubpasses = nullptr;
	AttachmentRef* m_pRefs = nullptr;
	ShadingRateAttachmentRef* m_pSRRefs = nullptr;
	BarrierData *m_pBarrierData = nullptr;
};
BV_CREATE_CAST_TO_D3D12(BvRenderPass)
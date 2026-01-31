#pragma once


#include "BvCommonVk.h"
#include "BDeV/Core/RenderAPI/BvRenderPass.h"
#include "BDeV/Core/Container/BvRobinMap.h"


class BvRenderDeviceVk;


class BvRenderPassVk final : public IBvRenderPass, public IBvResourceVk
{
public:
	BvRenderPassVk(BvRenderDeviceVk* pDevice, const RenderPassDesc& renderPassDesc, VkRenderPass renderPass);
	~BvRenderPassVk();

	BV_INLINE const RenderPassDesc& GetDesc() const override { return m_RenderPassDesc; }
	BV_INLINE const VkRenderPass GetHandle() const { return m_RenderPass; }
	BV_INLINE bool IsValid() const { return m_RenderPass != VK_NULL_HANDLE; }

private:
	void Destroy();

private:
	BvRenderDeviceVk* m_pDevice = nullptr;
	VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	RenderPassDesc m_RenderPassDesc;
};
BV_OBJECT_DEFINE_ID(BvRenderPassVk, "1be279e7-57e1-49a2-98f0-61bb54edac9b");
BV_CREATE_CAST_TO_VK(BvRenderPass)
#pragma once


#include "BvRenderGl/BvCommonGl.h"
#include "BDeV/RenderAPI/BvRenderPass.h"


class BvRenderDeviceGl;


class BvRenderPassGl final : public BvRenderPass
{
public:
	BvRenderPassGl(const BvRenderDeviceGl& device, const RenderPassDesc& renderPassDesc);
	~BvRenderPassGl();

private:
	const BvRenderDeviceGl& m_Device;
};
#pragma once


#include "BDeV/Core/RenderAPI/BvShaderResource.h"
#include "BDeV/Core/Container/BvRobinMap.h"
#include "BvCommonD3D12.h"


class BvRenderDeviceD3D12;


class BvShaderResourceLayoutD3D12 : public IBvShaderResourceLayout, public IBvResourceD3D12
{
public:
	BvShaderResourceLayoutD3D12(BvRenderDeviceD3D12* pDevice, const ShaderResourceLayoutCreateDesc& srlDesc);
	~BvShaderResourceLayoutD3D12();

	BV_INLINE const ShaderResourceLayoutCreateDesc& GetDesc() const { return m_ShaderResourceLayoutDesc; }
	BV_INLINE auto& GetRootParams() const { return m_RootParams; }
	BV_INLINE bool IsBindless(u32 rootParamIndex) const { return m_RootParamsBindlessFlags[rootParamIndex]; }

	u32 GetRootSignatureSlot(u32 binding, u32 set) const;

	struct ResourceId
	{
		u32 m_Binding;
		u32 m_Set;

		friend bool operator==(const ResourceId& lhs, const ResourceId& rhs)
		{
			return lhs.m_Binding == rhs.m_Binding && lhs.m_Set == rhs.m_Set;
		}

		friend bool operator<(const ResourceId& lhs, const ResourceId& rhs)
		{
			return lhs.m_Set <= rhs.m_Set && lhs.m_Binding < rhs.m_Binding;
		}
	};

private:
	void Destroy();

private:
	BvRenderDeviceD3D12* m_pDevice = nullptr;
	ShaderResourceLayoutCreateDesc m_ShaderResourceLayoutDesc;
	BvVector<CD3DX12_ROOT_PARAMETER1> m_RootParams;
	BvVector<CD3DX12_DESCRIPTOR_RANGE1> m_Ranges;
	BvVector<bool> m_RootParamsBindlessFlags;
	BvRobinMap<ResourceId, u32> m_RootSignatureMap;
};


BV_CREATE_CAST_TO_D3D12(BvShaderResourceLayout)
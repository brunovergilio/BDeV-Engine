#pragma once


#include "BDeV/Core/Utils/BvUtils.h"
#include "BvSwapChain.h"
#include "BvRenderPass.h"
#include "BvShaderResource.h"
#include "BvPipelineState.h"
#include "BvBuffer.h"
#include "BvBufferView.h"
#include "BvTexture.h"
#include "BvTextureView.h"
#include "BvSampler.h"
#include "BvQuery.h"
#include "BvCommandContext.h"
#include "BDeV/Reflection/BvRTTI.h"
#include "BDeV/Core/System/Threading/BvSync.h"
#include <functional>


class BvRenderDevice
{
	BV_NOCOPYMOVE(BvRenderDevice);

public:
	virtual BvSwapChain* CreateSwapChain(BvWindow* pWindow, const SwapChainDesc& desc, BvCommandContext* pContext) = 0;
	virtual BvBuffer* CreateBuffer(const BufferDesc& desc, const BufferInitData* pInitData = nullptr) = 0;
	virtual BvBufferView* CreateBufferView(const BufferViewDesc& desc) = 0;
	virtual BvTexture* CreateTexture(const TextureDesc& desc, const TextureInitData* pInitData = nullptr) = 0;
	virtual BvTextureView* CreateTextureView(const TextureViewDesc& desc) = 0;
	virtual BvSampler* CreateSampler(const SamplerDesc& desc) = 0;
	virtual BvRenderPass* CreateRenderPass(const RenderPassDesc& renderPassDesc) = 0;
	virtual BvShaderResourceLayout* CreateShaderResourceLayout(u32 shaderResourceCount, const ShaderResourceDesc* pShaderResourceDescs,
		const ShaderResourceConstantDesc& shaderResourceConstantDesc) = 0;
	virtual BvGraphicsPipelineState* CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc) = 0;
	virtual BvQuery* CreateQuery(QueryType queryType) = 0;

	virtual void WaitIdle() const = 0;

	virtual BvCommandContext* GetGraphicsContext(u32 index = 0) const = 0;
	virtual BvCommandContext* GetComputeContext(u32 index = 0) const = 0;
	virtual BvCommandContext* GetTransferContext(u32 index = 0) const = 0;

	virtual void GetCopyableFootprints(const TextureDesc& textureDesc, u32 subresourceCount, SubresourceFootprint* pSubresources, u64* pTotalBytes = nullptr) const = 0;

	virtual bool SupportsQueryType(QueryType queryType, QueueFamilyType commandType) const = 0;
	virtual bool IsFormatSupported(Format format) const = 0;

protected:
	BvRenderDevice() {}
	virtual ~BvRenderDevice() = 0 {};
};



class BvRenderObjPool
{
public:
	template<typename Type>
	void Init()
	{
		m_CleanupFn = [this]()
		{
			for (auto pMem : m_Free)
			{
				auto pObj = reinterpret_cast<Type*>(pMem);
				delete pObj;
			}
			m_Free.Clear();
			for (auto pMem : m_Used)
			{
				auto pObj = reinterpret_cast<Type*>(pMem);
				delete pObj;
			}
			m_Used.Clear();
		};
	}

	template<typename Type, typename... Args>
	Type* Create(Args&&... args)
	{
		Type* pObj = nullptr;
		{
			BvScopedLock lock(m_FreeLock);
			if (m_Free.Size() > 0)
			{
				auto pMem = m_Free.Back();
				m_Free.PopBack();
				pObj = new(pMem) Type(std::forward<Args>(args)...);
			}
		}
		if (!pObj)
		{
			pObj = new Type(std::forward<Args>(args)...);
		}
		{
			BvScopedLock lock(m_UsedLock);
			m_Used.EmplaceBack(pObj);
		}

		return pObj;
	}

	template<typename Type>
	void Destroy(Type* pObj)
	{
		pObj->~Type();

		{
			BvScopedLock lock(m_UsedLock);
			for (auto i = 0u; i < m_Used.Size(); i++)
			{
				if (pObj == m_Used[i])
				{
					std::swap(m_Used[i], m_Used.Back());
					m_Used.PopBack();
					break;
				}
			}
		}

		{
			BvScopedLock lock(m_FreeLock);
			m_Free.EmplaceBack(pObj);
		}
	}

	void DestroyAll()
	{
		m_CleanupFn();
	}

	BvRenderObjPool() {}
	~BvRenderObjPool()
	{
		if (m_CleanupFn)
		{
			m_CleanupFn();
		}
	}

private:
	std::function<void()> m_CleanupFn;
	BvVector<void*> m_Free;
	BvVector<void*> m_Used;
	BvSpinlock m_FreeLock;
	BvSpinlock m_UsedLock;
};


class BvRenderDeviceFactory
{
public:
	BvRenderDeviceFactory() {}
	~BvRenderDeviceFactory()
	{
		for (auto&& obj : m_ObjPools)
		{
			delete obj.second;
		}
	}

	template<typename Type, typename... Args>
	Type* Create(Args&&... args)
	{
		//BvScopedLock lock(m_Lock);
		BvRenderObjPool* pObjPool = nullptr;
		auto typeId = GetTypeIdT<Type>();
		auto it = m_ObjPools.FindKey(typeId);
		if (it == m_ObjPools.cend())
		{
			pObjPool = new BvRenderObjPool();
			pObjPool->Init<Type>();
			auto pool = m_ObjPools.Emplace(typeId, pObjPool);
		}
		else
		{
			pObjPool = it->second;
		}

		return pObjPool->Create<Type>(std::forward<Args>(args)...);
	}

	template<typename Type>
	void Destroy(Type* pObj)
	{
		m_ObjPools[GetTypeIdT<Type>()]->Destroy(pObj);
	}

	template<typename Type>
	void DestroyAllOfType()
	{
		auto typeId = GetTypeIdT<Type>();
		if (m_ObjPools.HasKey(typeId))
		{
			m_ObjPools[typeId]->DestroyAll();
		}
	}
private:
	BvRobinMap<u64, BvRenderObjPool*> m_ObjPools;
	BvSpinlock m_Lock;
};
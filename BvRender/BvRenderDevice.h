#pragma once


#include "BvCore/Utils/BvUtils.h"
#include "BvRender/BvCommandQueue.h"
#include "BvRender/BvSwapChain.h"
#include "BvRender/BvRenderPass.h"
#include "BvRender/BvCommandPool.h"
#include "BvRender/BvShaderResource.h"
#include "BvRender/BvPipelineState.h"
#include "BvRender/BvBuffer.h"
#include "BvRender/BvBufferView.h"
#include "BvRender/BvTexture.h"
#include "BvRender/BvTextureView.h"
#include "BvCore/Utils/RTTI.h"


#if defined(CreateSemaphore)
#undef CreateSemaphore
#endif


class BvSemaphore;


class BvRenderDevice
{
	BV_NOCOPYMOVE(BvRenderDevice);

public:
	virtual BvSwapChain* CreateSwapChain(const SwapChainDesc& desc, BvCommandQueue& commandQueue) = 0;
	virtual BvBuffer* CreateBuffer(const BufferDesc& desc) = 0;
	virtual BvBufferView* CreateBufferView(const BufferViewDesc& desc) = 0;
	virtual BvTexture* CreateTexture(const TextureDesc& desc) = 0;
	virtual BvTextureView* CreateTextureView(const TextureViewDesc& desc) = 0;
	virtual BvSemaphore* CreateSemaphore(const u64 initialValue = 0) = 0;
	// CreateQueryPool
	// CreateQuery
	virtual BvRenderPass* CreateRenderPass(const RenderPassDesc& renderPassDesc) = 0;
	virtual BvCommandPool* CreateCommandPool(const CommandPoolDesc& commandPoolDesc = CommandPoolDesc()) = 0;
	virtual BvShaderResourceLayout* CreateShaderResourceLayout(const ShaderResourceLayoutDesc& shaderResourceLayoutDesc =
		ShaderResourceLayoutDesc()) = 0;
	virtual BvShaderResourceSetPool* CreateShaderResourceSetPool(const ShaderResourceSetPoolDesc& shaderResourceSetPoolDesc =
		ShaderResourceSetPoolDesc()) = 0;
	virtual BvGraphicsPipelineState* CreateGraphicsPipeline(const GraphicsPipelineStateDesc& graphicsPipelineStateDesc) = 0;

	virtual void WaitIdle() const = 0;

	virtual BvCommandQueue* GetGraphicsQueue(const u32 index = 0) const = 0;
	virtual BvCommandQueue* GetComputeQueue(const u32 index = 0) const = 0;
	virtual BvCommandQueue* GetTransferQueue(const u32 index = 0) const = 0;
	virtual bool QueueFamilySupportsPresent(const QueueFamilyType queueFamilyType) const = 0;

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
			for (auto pMem : m_Used)
			{
				auto pObj = reinterpret_cast<Type*>(pMem);
				delete pObj;
			}
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
		BvScopedLock lock(m_Lock);
		BvRenderObjPool* pObjPool = nullptr;
		auto typeId = GetTypeId<Type>();
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
		m_ObjPools[GetTypeId<Type>()]->Destroy(pObj);
	}
private:
	BvRobinMap<u64, BvRenderObjPool*> m_ObjPools;
	BvSpinlock m_Lock;
};
#pragma once


#include "BvCommonD3D12.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvRobinMap.h"
#include "BDeV/Core/System/Threading/BvSync.h"


class BvRenderDeviceD3D12;
class BvShaderResourceLayoutD3D12;


class BvDescriptorHandle
{
public:
    BV_INLINE BvDescriptorHandle()
    {
    }

    BV_INLINE BvDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle = D3D12_GPU_DESCRIPTOR_HANDLE{ kInvalidPos })
        : m_CPUHandle(CPUHandle), m_GPUHandle(GPUHandle)
    {
    }

    BV_INLINE BvDescriptorHandle GetByIndex(i64 index, u32 descriptorSize)
    {
        return { { m_CPUHandle.ptr + index * descriptorSize }, { IsShaderVisible() ? m_GPUHandle.ptr + index * descriptorSize : kInvalidPos } };
    }

    BV_INLINE BvDescriptorHandle operator+(i64 offset) const
    {
        return { { m_CPUHandle.ptr + offset }, { IsShaderVisible() ? m_GPUHandle.ptr + offset : kInvalidPos } };
    }

    BV_INLINE void operator+=(i64 offset)
    {
        if (m_CPUHandle.ptr != kInvalidPos)
        {
            m_CPUHandle.ptr += offset;
        }
        if (m_GPUHandle.ptr != kInvalidPos)
        {
            m_GPUHandle.ptr += offset;
        }
    }

    //const D3D12_CPU_DESCRIPTOR_HANDLE* operator&() const { return &m_CPUHandle; }
    BV_INLINE operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return m_CPUHandle; }
    BV_INLINE operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return m_GPUHandle; }

    BV_INLINE auto GetCPUHandle() const { return m_CPUHandle.ptr; }
	BV_INLINE auto GetGPUHandle() const { return m_GPUHandle.ptr; }
    BV_INLINE operator bool() const { return m_CPUHandle.ptr != kInvalidPos; }
    BV_INLINE bool IsShaderVisible() const { return m_GPUHandle.ptr != kInvalidPos; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle{kInvalidPos};
	D3D12_GPU_DESCRIPTOR_HANDLE m_GPUHandle{kInvalidPos};
};


// GPU Descriptor Heaps will have all its handles in a single ID3D12DescriptorHeap object, meaning there will
// be only 2 BvGPUDescriptorHeapD3D12 objects, one for CBV_SRV_UAV types and another for Sampler types.
class BvGPUDescriptorHeapD3D12
{
public:
	BvGPUDescriptorHeapD3D12(BvRenderDeviceD3D12* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 numDescriptors);
	~BvGPUDescriptorHeapD3D12();

    BvDescriptorHandle Allocate(u32 count = 1);

    BV_INLINE u32 GetIndex(const BvDescriptorHandle& handle) const
    {
        return (handle.GetCPUHandle() - m_Handle.GetCPUHandle()) / m_DescriptorSize;
    }

	BV_INLINE u32 GetDescriptorSize() const { return m_DescriptorSize; }
	BV_INLINE ID3D12DescriptorHeap* GetHandle() const { return m_Heap.Get(); }

private:
	ComPtr<ID3D12DescriptorHeap> m_Heap;
	BvDescriptorHandle m_Handle;
	u32 m_DescriptorSize = 0;
	u32 m_NumDescriptors = 0;
    std::atomic<u32> m_CurrUsed = 0;
};


class BvCPUDescriptorHeapD3D12
{
public:
    BvCPUDescriptorHeapD3D12(BvRenderDeviceD3D12* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 numDescriptorsPerHeap);
    ~BvCPUDescriptorHeapD3D12();

    D3D12_CPU_DESCRIPTOR_HANDLE Allocate(u32 count = 1);

	BV_INLINE u32 GetDescriptorSize() const { return m_DescriptorSize; }
    BV_INLINE D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return m_HeapType; }

private:
    BvRenderDeviceD3D12* m_pDevice = nullptr;
	BvVector<ID3D12DescriptorHeap*> m_CPUHeaps;
    D3D12_CPU_DESCRIPTOR_HANDLE m_Handle{};
	u32 m_DescriptorSize = 0;
	u32 m_NumDescriptors = 0;
    u32 m_CurrUsed = 0;
    D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
};


class BvDescriptorPoolD3D12
{
public:
    BvDescriptorPoolD3D12();
    BvDescriptorPoolD3D12(BvDescriptorPoolD3D12&&) noexcept = default;
    BvDescriptorPoolD3D12& operator=(BvDescriptorPoolD3D12&&) noexcept = default;
    BvDescriptorPoolD3D12(BvRenderDeviceD3D12* pDevice, const BvShaderResourceLayoutD3D12* pLayout, u32 rootIndex, u32 maxAllocationsPerPool);
    ~BvDescriptorPoolD3D12();

	BvDescriptorHandle Allocate();
    void RecycleDescriptor(const BvDescriptorHandle& handle);
	void Reset();

    BV_INLINE bool IsValid() const { return m_pDescriptorHeap != nullptr; }
    BV_INLINE u32 GetHandleCount() const { return m_HandleCount; }
    BV_INLINE auto GetHeapType() const { return m_HeapType; }
    BV_INLINE auto GetHandleSize() const { return m_pDescriptorHeap->GetDescriptorSize(); }

private:
    struct PoolData
    {
        BvDescriptorHandle m_Handle;
        u32 m_CurrAllocations = 0;
    };

    BvGPUDescriptorHeapD3D12* m_pDescriptorHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
    BvVector<PoolData> m_Pools;
	BvVector<BvDescriptorHandle> m_FreeDescriptors;
	u32 m_RootIndex = 0;
	u32 m_MaxAllocationsPerPool = 0;
	u32 m_CurrPoolIndex = 0;
    u32 m_HandleCount = 0;
};


struct ResourceIdD3D12
{
    u32 m_RootParamIndex;
	u32 m_RegisterSpace;
	u32 m_Binding;
	u32 m_ArrayIndex;

	friend bool operator==(const ResourceIdD3D12& lhs, const ResourceIdD3D12& rhs)
	{
		return lhs.m_RootParamIndex == rhs.m_RootParamIndex && lhs.m_RegisterSpace == rhs.m_RegisterSpace
            && lhs.m_Binding == rhs.m_Binding && lhs.m_ArrayIndex == rhs.m_ArrayIndex;
	}
};


class BvResourceBindingStateD3D12 final
{
public:
	BvResourceBindingStateD3D12() = default;
	~BvResourceBindingStateD3D12() = default;

	void SetResource(const BvDescriptorHandle& handle, u32 rootParamIndex, u32 registerSpace, u32 binding, u32 arrayIndex);

	void Reset();

	const BvDescriptorHandle* GetResource(const ResourceIdD3D12& resId) const;

	BV_INLINE bool IsEmpty() const { return m_Descriptors.Empty(); }
	BV_INLINE bool IsDirty(u32 rootParamIndex) const { return m_DirtyRootParams[rootParamIndex]; }
	BV_INLINE void MarkClean(u32 rootParamIndex) { m_DirtyRootParams[rootParamIndex] = false; }

private:
	std::pair<BvDescriptorHandle*, bool> AddOrRetrieveResourceData(u32 rootParamIndex, u32 registerSpace, u32 binding, u32 arrayIndex);

private:
	BvRobinMap<ResourceIdD3D12, u32> m_Bindings;
	BvVector<BvDescriptorHandle> m_Descriptors;
	BvVector<bool> m_DirtyRootParams;
};
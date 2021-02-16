#pragma once


#include "BvRender/BvCommandQueue.h"
#include "BvCore/Utils/BvUtils.h"


class BvCommandBuffer;


struct CommandPoolDesc
{
	QueueFamilyType m_QueueFamilyType = QueueFamilyType::kGraphics;
	CommandType m_CommandType = CommandType::kPrimary;
};


class BvCommandPool
{
	BV_NOCOPYMOVE(BvCommandPool);

public:
	virtual void AllocateCommandBuffers(const u32 commandBufferCount, BvCommandBuffer ** ppCommandBuffers) = 0;
	virtual void FreeCommandBuffers(const u32 commandBufferCount, BvCommandBuffer ** ppCommandBuffers) = 0;

	virtual void Reset() = 0;

	BvCommandBuffer * AllocateCommandBuffer()
	{
		BvCommandBuffer * pCB = nullptr;
		AllocateCommandBuffers(1, &pCB);
		
		return pCB;
	}

	void FreeCommandBuffer(BvCommandBuffer * & pCommandBuffer)
	{
		FreeCommandBuffers(1, &pCommandBuffer);
		pCommandBuffer = nullptr;
	}

	BV_INLINE const CommandPoolDesc & GetDesc() const { return m_CommandPoolDesc; }

protected:
	BvCommandPool(const CommandPoolDesc & commandPoolDesc = CommandPoolDesc())
		: m_CommandPoolDesc(commandPoolDesc) {}
	virtual ~BvCommandPool() = 0 {}

protected:
	CommandPoolDesc m_CommandPoolDesc;
};
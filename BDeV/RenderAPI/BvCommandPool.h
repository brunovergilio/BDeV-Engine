#pragma once


#include "BvCommandQueue.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Utils/BvEnum.h"


class BvCommandBuffer;


enum class CommandPoolFlags : u8
{
	// No flags
	kNone = 0,

	// Makes the command pool send every command buffer from the FreeCommandBuffers()
	// call to a free list instead of freeing them, so they can be reused later
	// when AllocateCommandBuffers() is called again
	kRecycleCommandBuffers = 1,

	// Indicates every command buffer allocated through the command pool will be
	// short lived, being either reset or freed in a short timeframe
	kTransient = 2
};
BV_USE_ENUM_CLASS_OPERATORS(CommandPoolFlags);


struct CommandPoolDesc
{
	QueueFamilyType m_QueueFamilyType = QueueFamilyType::kGraphics;
	CommandType m_CommandType = CommandType::kPrimary;
	CommandPoolFlags m_Flags = CommandPoolFlags::kNone;
};


class BvCommandPool
{
	BV_NOCOPYMOVE(BvCommandPool);

public:
	virtual void AllocateCommandBuffers(u32 commandBufferCount, BvCommandBuffer ** ppCommandBuffers) = 0;
	virtual void FreeCommandBuffers(u32 commandBufferCount, BvCommandBuffer ** ppCommandBuffers) = 0;

	virtual void Reset() = 0;

	BvCommandBuffer * AllocateCommandBuffer()
	{
		BvCommandBuffer * pCB = nullptr;
		AllocateCommandBuffers(1, &pCB);
		
		return pCB;
	}

	void FreeCommandBuffer(BvCommandBuffer * pCommandBuffer)
	{
		FreeCommandBuffers(1, &pCommandBuffer);
	}

	BV_INLINE const CommandPoolDesc & GetDesc() const { return m_CommandPoolDesc; }

protected:
	BvCommandPool(const CommandPoolDesc & commandPoolDesc = CommandPoolDesc())
		: m_CommandPoolDesc(commandPoolDesc) {}
	virtual ~BvCommandPool() = 0 {}

protected:
	CommandPoolDesc m_CommandPoolDesc;
};
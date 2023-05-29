#include "BvBufferGl.h"
#include "BvTypeConversionsGl.h"


BvBufferGl::BvBufferGl(const BvRenderDeviceVk& device, const BufferDesc& bufferDesc)
	: BvBuffer(bufferDesc)
{
	Create();
}


BvBufferGl::~BvBufferGl()
{
	Destroy();
}


void BvBufferGl::Create()
{
	GLenum mappingFlags = GetGlBufferMemoryFlags(m_BufferDesc.m_MemoryFlags);

	glCreateBuffers(1, &m_Buffer);
	glNamedBufferStorage(m_Buffer, m_BufferDesc.m_Size, nullptr, GL_DYNAMIC_STORAGE_BIT | mappingFlags);
}


void BvBufferGl::Destroy()
{
	if (m_Buffer)
	{
		glDeleteBuffers(1, &m_Buffer);
		m_Buffer = 0;
	}
}


void* const BvBufferGl::Map(const u64 size, const u64 offset)
{
	m_pMapped = glMapNamedBufferRange(m_Buffer, offset, size == 0 ? m_BufferDesc.m_Size : size, GetGlBufferMappingFlags(m_BufferDesc.m_MemoryFlags));

	return m_pMapped;
}


void BvBufferGl::Unmap()
{
	if (!glUnmapNamedBuffer(m_Buffer))
	{
		Destroy();
		Create();
	}
	m_pMapped = nullptr;
}


void BvBufferGl::Flush(const u64 size, const u64 offset) const
{
}


void BvBufferGl::Invalidate(const u64 size, const u64 offset) const
{
}
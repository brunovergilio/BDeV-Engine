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
	GLenum flags = GetGlBufferMemoryFlags(m_BufferDesc.m_MemoryType);
	GLsizeiptr size = (GLsizeiptr)m_BufferDesc.m_Size;

	auto [target, usage] = GetBufferUsageFlags();
	// Original
	//glGenBuffers(1, &m_Buffer);
	//glBindBuffer(target, m_Buffer);
	//glBufferData(target, size, nullptr, usage);

	// DSA
	glCreateBuffers(1, &m_Buffer);
	glNamedBufferStorage(m_Buffer, size, nullptr, GL_DYNAMIC_STORAGE_BIT | flags);

	glBindBuffer(target, 0);
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
	m_pMapped = glMapNamedBufferRange(m_Buffer, offset, size == 0 ? m_BufferDesc.m_Size : size, GetGlBufferMappingFlags(m_BufferDesc.m_MemoryType));

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


std::pair<GLenum, GLenum> BvBufferGl::GetBufferUsageFlags()
{
	GLenum target = 0;
	GLenum usage = 0;

	if ((m_BufferDesc.m_UsageFlags & BufferUsage::kVertexBuffer) == BufferUsage::kVertexBuffer)
	{
		target = GL_ARRAY_BUFFER;
		usage = GL_STATIC_DRAW;
	}
	else if ((m_BufferDesc.m_UsageFlags & BufferUsage::kIndexBuffer) == BufferUsage::kIndexBuffer)
	{
		target = GL_ELEMENT_ARRAY_BUFFER;
		usage = GL_STATIC_DRAW;
	}
	else if ((m_BufferDesc.m_UsageFlags & BufferUsage::kUniformBuffer) == BufferUsage::kUniformBuffer)
	{
		target = GL_UNIFORM_BUFFER;
		usage = GL_DYNAMIC_DRAW;
	}
	else if ((m_BufferDesc.m_UsageFlags & BufferUsage::kIndirectBuffer) == BufferUsage::kIndirectBuffer)
	{
		target = GL_DRAW_INDIRECT_BUFFER;
		usage = GL_STATIC_DRAW;
	}
	else if ((m_BufferDesc.m_UsageFlags & BufferUsage::kStorageBuffer) == BufferUsage::kStorageBuffer)
	{
		target = GL_SHADER_STORAGE_BUFFER;
		usage = GL_STATIC_DRAW;
	}
	else if ((m_BufferDesc.m_UsageFlags & BufferUsage::kUniformTexelBuffer) == BufferUsage::kUniformTexelBuffer)
	{
		target = GL_UNIFORM_BUFFER;
		usage = GL_STATIC_DRAW;
	}
	else if ((m_BufferDesc.m_UsageFlags & BufferUsage::kStorageTexelBuffer) == BufferUsage::kStorageTexelBuffer)
	{
		target = GL_SHADER_STORAGE_BUFFER;
		usage = GL_DYNAMIC_DRAW;
	}
	else if ((m_BufferDesc.m_UsageFlags & BufferUsage::kTransferSrc) == BufferUsage::kTransferSrc)
	{
		target = GL_COPY_READ_BUFFER;
		usage = GL_STREAM_COPY;
	}
	else if ((m_BufferDesc.m_UsageFlags & BufferUsage::kTransferDst) == BufferUsage::kTransferDst)
	{
		target = GL_COPY_WRITE_BUFFER;
		usage = GL_STREAM_COPY;
	}

	return std::make_pair(target, usage);
}
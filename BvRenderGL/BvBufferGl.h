#pragma once


#include "BDeV/RenderAPI/BvBuffer.h"
#include "BvCommonGl.h"


class BvRenderDeviceVk;


class BvBufferGl final : public BvBuffer
{
	BV_NOCOPYMOVE(BvBufferGl);

public:
	BvBufferGl(const BvRenderDeviceVk& device, const BufferDesc& bufferDesc);
	~BvBufferGl();

	void Create();
	void Destroy();

	void* const Map(const u64 size, const u64 offset) override;
	void Unmap() override;
	void Flush(const u64 size, const u64 offset) const override;
	void Invalidate(const u64 size, const u64 offset) const override;

	BV_INLINE GLuint GetHandle() const { return m_Buffer; }

private:
	std::pair<GLenum, GLenum> GetBufferUsageFlags();

private:
	GLuint m_Buffer = 0;
};
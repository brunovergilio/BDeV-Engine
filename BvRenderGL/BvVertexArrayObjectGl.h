#pragma once

#include "BvCommonGl.h"
#include "BDeV/Container/BvFixedVector.h"
#include "BDeV/Container/BvRobinMap.h"
#include "BDeV/System/Threading/BvSync.h"
#include "BDeV/RenderAPI/BvPipelineState.h"
#include "BvBufferViewGl.h"


struct VAODesc
{
	BvFixedVector<VertexInputDesc, kMaxVertexAttributes> m_VertexAttributeDesc;
	BvFixedVector<BvBufferViewGl*, kMaxVertexBuffers> m_VertexBuffers;
	BvBufferViewGl* m_pIndexBuffer = nullptr;

	bool operator==(const VAODesc& rhs) const
	{
		if (m_VertexAttributeDesc.Size() != rhs.m_VertexAttributeDesc.Size()
			|| m_VertexBuffers.Size() != rhs.m_VertexBuffers.Size() ||  m_pIndexBuffer != rhs.m_pIndexBuffer)
		{
			return false;
		}

		for (auto i = 0u; i < m_VertexAttributeDesc.Size(); i++)
		{
			if (m_VertexAttributeDesc[i] != rhs.m_VertexAttributeDesc[i])
			{
				return false;
			}
		}

		for (auto i = 0u; i < m_VertexBuffers.Size(); i++)
		{
			if (m_VertexBuffers[i] != rhs.m_VertexBuffers[i])
			{
				return false;
			}
		}

		return true;
	}
};


template<>
struct std::hash<VAODesc>
{
	size_t operator()(const VAODesc& val) const
	{
		u64 hash = 0;
		HashCombine(hash, val.m_VertexAttributeDesc.Size());
		for (auto i = 0; i < val.m_VertexAttributeDesc.Size(); i++)
		{
			HashCombine(hash, val.m_VertexAttributeDesc[i]);
		}

		HashCombine(hash, val.m_VertexBuffers.Size());
		for (auto i = 0; i < val.m_VertexBuffers.Size(); i++)
		{
			HashCombine(hash, val.m_VertexBuffers[i]);
		}

		HashCombine(hash, val.m_pIndexBuffer);

		return hash;
	}
};


class BvVertexArrayObjectGl
{
	BV_NOCOPYMOVE(BvVertexArrayObjectGl);

public:
	BvVertexArrayObjectGl(const VAODesc& vaoDesc);
	~BvVertexArrayObjectGl();

	void Create();
	void Destroy();

private:
	VAODesc m_VAODesc{};
	GLuint m_VAO = 0;
};


class BvVAOManagerGl
{
	BV_NOCOPYMOVE(BvVAOManagerGl);

public:
	BvVAOManagerGl();
	~BvVAOManagerGl();

	BvVertexArrayObjectGl* GetVAO(const VAODesc& desc);
	void RemoveVAO(const BvBufferViewGl* const pBufferView);
	void Destroy();

private:
	BvRobinMap<VAODesc, BvVertexArrayObjectGl*> m_VAOs;
	BvSpinlock m_Lock;
};
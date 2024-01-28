#include "BvVertexArrayObjectGl.h"
#include "BvTypeConversionsGl.h"


BvVertexArrayObjectGl::BvVertexArrayObjectGl(const VAODesc& vaoDesc)
	: m_VAODesc(vaoDesc)
{
	Create();
}


BvVertexArrayObjectGl::~BvVertexArrayObjectGl()
{
	Destroy();
}


void BvVertexArrayObjectGl::Create()
{
	// Original
	//glGenVertexArrays(1, &m_VAO);
	//glBindVertexArray(m_VAO);
	//for (auto i = 0u; i < m_VAODesc.m_VertexAttributeDesc.Size(); i++)
	//{
	//	const auto& attribute = m_VAODesc.m_VertexAttributeDesc[i];
	//	const auto& formatMap = GetGlFormatMap(attribute.m_Format);
	//	auto pBufferView = m_VAODesc.m_VertexBuffers[attribute.m_Binding];
	//	BvAssert(pBufferView != nullptr, "Referenced buffer view can't be nullptr");
	//	auto buffer = reinterpret_cast<BvBufferGl*>(pBufferView->GetBuffer())->GetHandle();
	//	
	//	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	//	glEnableVertexAttribArray(attribute.m_Location);
	//	if (formatMap.type == GL_FLOAT || formatMap.type == GL_HALF_FLOAT)
	//	{
	//		bool normalized = formatMap.normalized == GL_TRUE || attribute.m_Normalized;
	//		glVertexAttribPointer(attribute.m_Location, formatMap.componentCount, formatMap.type,
	//			normalized, attribute.m_Stride, reinterpret_cast<const void*>(attribute.m_Offset));
	//	}
	//	else
	//	{
	//		glVertexAttribIPointer(attribute.m_Location, formatMap.componentCount, formatMap.type,
	//			attribute.m_Stride, reinterpret_cast<const void*>(attribute.m_Offset));
	//	}

	//	if (attribute.m_InputRate == InputRate::kPerInstance)
	//	{
	//		glVertexAttribDivisor(attribute.m_Location, attribute.m_InstanceRate);
	//	}
	//}
	//if (m_VAODesc.m_pIndexBuffer)
	//{
	//	auto buffer = reinterpret_cast<BvBufferGl*>(m_VAODesc.m_pIndexBuffer->GetBuffer())->GetHandle();
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	//}

	// DSA
	glCreateVertexArrays(1, &m_VAO);
	for (auto i = 0u; i < m_VAODesc.m_VertexAttributeDesc.Size(); i++)
	{
		const auto& attribute = m_VAODesc.m_VertexAttributeDesc[i];
		const auto& formatMap = GetGlFormatMap(attribute.m_Format);
		auto pBufferView = m_VAODesc.m_VertexBuffers[attribute.m_Binding];
		BvAssert(pBufferView != nullptr, "Referenced buffer view can't be nullptr");
		auto buffer = reinterpret_cast<BvBufferGl*>(pBufferView->GetBuffer())->GetHandle();

		GLsizei stride = pBufferView->GetDesc().m_Stride;
		glVertexArrayVertexBuffer(m_VAO, m_VAODesc.m_VertexAttributeDesc[i].m_Binding, buffer, pBufferView->GetDesc().m_Offset, stride);
		glEnableVertexArrayAttrib(m_VAO, attribute.m_Location);
		if (formatMap.type == GL_FLOAT || formatMap.type == GL_HALF_FLOAT)
		{
			bool normalized = formatMap.normalized == GL_TRUE || attribute.m_Normalized;
			glVertexArrayAttribFormat(m_VAO, attribute.m_Location, formatMap.componentCount, formatMap.type, normalized, attribute.m_Offset);
		}
		else
		{
			glVertexArrayAttribIFormat(m_VAO, attribute.m_Location, formatMap.componentCount, formatMap.type, attribute.m_Offset);
		}
		glVertexArrayAttribBinding(m_VAO, attribute.m_Location, attribute.m_Binding);

		if (attribute.m_InputRate == InputRate::kPerInstance)
		{
			glVertexBindingDivisor(attribute.m_Binding, attribute.m_InstanceRate);
		}
	}
	if (m_VAODesc.m_pIndexBuffer)
	{
		auto buffer = reinterpret_cast<BvBufferGl*>(m_VAODesc.m_pIndexBuffer->GetBuffer())->GetHandle();
		glVertexArrayElementBuffer(m_VAO, buffer);
	}

	// Clear states
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void BvVertexArrayObjectGl::Destroy()
{
	if (m_VAO)
	{
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
	}
}


BvVAOManagerGl::BvVAOManagerGl()
{
}


BvVAOManagerGl::~BvVAOManagerGl()
{
	Destroy();
}


BvVertexArrayObjectGl* BvVAOManagerGl::GetVAO(const VAODesc& desc)
{
	BvScopedLock lock(m_Lock);
	decltype(auto) pVAO = m_VAOs[desc];

	if (pVAO == nullptr)
	{
		pVAO = new BvVertexArrayObjectGl(desc);
	}

	return pVAO;
}


void BvVAOManagerGl::RemoveVAO(const BvBufferViewGl* const pBufferView)
{
	BvScopedLock lock(m_Lock);
	for (const auto& vao : m_VAOs)
	{
		const auto& desc = vao.first;
		for (auto pView : desc.m_VertexBuffers)
		{
			if (pView == pBufferView)
			{
				m_VAOs.Erase(desc);
				break;
			}
		}

		if (desc.m_pIndexBuffer != nullptr && desc.m_pIndexBuffer == pBufferView)
		{
			m_VAOs.Erase(desc);
		}
	}
}


void BvVAOManagerGl::Destroy()
{
	for (auto&& vao : m_VAOs)
	{
		delete vao.second;
	}
	m_VAOs.Clear();
}
#include "BvFramebufferGl.h"
#include "BvTextureViewGl.h"
#include "BvTypeConversionsGl.h"


BvFramebufferGl::BvFramebufferGl(const FramebufferDesc& framebufferDesc)
	: m_FramebufferDesc(framebufferDesc)
{
	Create();
}


BvFramebufferGl::~BvFramebufferGl()
{
	Destroy();
}


void BvFramebufferGl::Create()
{
	// Original
	//glGenFramebuffers(1, &m_Framebuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

	// DSA
	glCreateFramebuffers(1, &m_Framebuffer);

	for (auto i = 0u; i < m_FramebufferDesc.m_RenderTargetViews.Size(); i++)
	{
		AttachToFramebuffer(GL_COLOR_ATTACHMENT0 + i, static_cast<BvTextureViewGl*>(m_FramebufferDesc.m_RenderTargetViews[i]));
	}

	if (m_FramebufferDesc.m_pDepthStencilView)
	{
		auto pView = static_cast<BvTextureViewGl*>(m_FramebufferDesc.m_pDepthStencilView);
		auto pTexture = pView->GetTexture();

		GLenum depthStencilAttachment = 0;
		switch (pTexture->GetInternalFormat())
		{
		case GL_DEPTH32F_STENCIL8:
		case GL_DEPTH24_STENCIL8:
			depthStencilAttachment = GL_DEPTH_STENCIL_ATTACHMENT;
			break;
		case GL_DEPTH_COMPONENT32F:
		case GL_DEPTH_COMPONENT16:
			depthStencilAttachment = GL_DEPTH_ATTACHMENT;
			break;
		}

		AttachToFramebuffer(depthStencilAttachment, pView);
	}
}


void BvFramebufferGl::Destroy()
{
	if (m_Framebuffer)
	{
		glDeleteFramebuffers(1, &m_Framebuffer);
		m_Framebuffer = 0;
	}
}


void BvFramebufferGl::AttachToFramebuffer(GLenum attachment, BvTextureViewGl* pView)
{
	const auto& viewDesc = pView->GetDesc();
	auto pTexture = pView->GetTexture();

	auto target = GetGlTextureTarget(pTexture->GetDesc());
	auto texture = pTexture->GetHandle();
	auto level = (GLint)viewDesc.m_SubresourceDesc.firstMip;
	auto layer = (GLint)viewDesc.m_SubresourceDesc.firstLayer;
	auto layerCount = (GLint)viewDesc.m_SubresourceDesc.layerCount;

	// In OpenGL, we can only bind a single layer or all layers, so if a specific range is
	// requested, only the first layer is used

	// Original
	//static constexpr GLenum kCubeMapTargets[6] =
	//{
	//	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	//	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	//	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	//	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	//	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	//	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	//};
	//switch (target)
	//{
	//case GL_TEXTURE_1D:
	//	glFramebufferTexture1D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_1D, texture, level);
	//	break;
	//case GL_TEXTURE_1D_ARRAY:
	//	if (layerCount == pTexture->GetDesc().m_Size.height)
	//	{
	//		glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture, level);
	//	}
	//	else if (layerCount == 1)
	//	{
	//		glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, texture, level, layer);
	//	}
	//	break;
	//case GL_TEXTURE_2D:
	//case GL_TEXTURE_2D_MULTISAMPLE:
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, level);
	//	break;
	//case GL_TEXTURE_2D_ARRAY:
	//case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
	//	if (layerCount == pTexture->GetDesc().m_Size.depthOrLayerCount)
	//	{
	//		glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture, level);
	//	}
	//	else if (layerCount == 1)
	//	{
	//		glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, texture, level, layer);
	//	}
	//	break;
	//case GL_TEXTURE_CUBE_MAP:
	//	if (layerCount == pTexture->GetDesc().m_Size.depthOrLayerCount)
	//	{
	//		glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture, level);
	//	}
	//	else if (layerCount == 1)
	//	{
	//		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, kCubeMapTargets[layer], texture, level);
	//	}
	//case GL_TEXTURE_CUBE_MAP_ARRAY:
	//	if (layerCount == pTexture->GetDesc().m_Size.depthOrLayerCount)
	//	{
	//		glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture, level);
	//	}
	//	else if (layerCount == 1)
	//	{
	//		glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, texture, level, layer);
	//	}
	//case GL_TEXTURE_3D:
	//	if (layerCount == pTexture->GetDesc().m_Size.depthOrLayerCount)
	//	{
	//		glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture, level);
	//	}
	//	else if (layerCount == 1)
	//	{
	//		glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, texture, level, layer);
	//	}
	//	break;
	//}

	// DSA
	switch (target)
	{
	case GL_TEXTURE_1D:
		glNamedFramebufferTexture(m_Framebuffer, attachment, texture, level);
		break;
	case GL_TEXTURE_1D_ARRAY:
		if (layerCount == (GLint)pTexture->GetDesc().m_Size.height)
		{
			glNamedFramebufferTexture(m_Framebuffer, attachment, texture, level);
		}
		else if (layerCount == 1)
		{
			glNamedFramebufferTextureLayer(m_Framebuffer, attachment, texture, level, layer);
		}
		break;
	case GL_TEXTURE_2D:
	case GL_TEXTURE_2D_MULTISAMPLE:
		glNamedFramebufferTexture(m_Framebuffer, attachment, texture, level);
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		if (layerCount == (GLint)pTexture->GetDesc().m_Size.depth)
		{
			glNamedFramebufferTexture(m_Framebuffer, attachment, texture, level);
		}
		else if (layerCount == 1)
		{
			glNamedFramebufferTextureLayer(m_Framebuffer, attachment, texture, level, layer);
		}
		break;
	case GL_TEXTURE_CUBE_MAP:
		if (layerCount == (GLint)pTexture->GetDesc().m_Size.depth)
		{
			glNamedFramebufferTexture(m_Framebuffer, attachment, texture, level);
		}
		else if (layerCount == 1)
		{
			glNamedFramebufferTextureLayer(m_Framebuffer, attachment, texture, level, layer);
		}
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		if (layerCount == (GLint)pTexture->GetDesc().m_Size.depth)
		{
			glNamedFramebufferTexture(m_Framebuffer, attachment, texture, level);
		}
		else if (layerCount == 1)
		{
			glNamedFramebufferTextureLayer(m_Framebuffer, attachment, texture, level, layer);
		}
	case GL_TEXTURE_3D:
		if (layerCount == (GLint)pTexture->GetDesc().m_Size.depth)
		{
			glNamedFramebufferTexture(m_Framebuffer, attachment, texture, level);
		}
		else if (layerCount == 1)
		{
			glNamedFramebufferTextureLayer(m_Framebuffer, attachment, texture, level, layer);
		}
		break;
	}
}


BvFramebufferManagerGl::BvFramebufferManagerGl()
{
}


BvFramebufferManagerGl::~BvFramebufferManagerGl()
{
	Destroy();
}


BvFramebufferGl* BvFramebufferManagerGl::GetFramebuffer(const FramebufferDesc& desc)
{
	BvScopedLock lock(m_Lock);
	decltype(auto) pFramebuffer = m_Framebuffers[desc];

	if (pFramebuffer == nullptr)
	{
		pFramebuffer = new BvFramebufferGl(desc);
	}

	return pFramebuffer;
}


void BvFramebufferManagerGl::RemoveFramebuffer(const BvTextureViewGl* const pTextureView)
{
	BvScopedLock lock(m_Lock);
	for (const auto& pFramebuffer : m_Framebuffers)
	{
		const auto& desc = pFramebuffer.first;
		for (auto pView : desc.m_RenderTargetViews)
		{
			if (pView == pTextureView)
			{
				m_Framebuffers.Erase(pFramebuffer.first);
				break;
			}
		}

		if (desc.m_pDepthStencilView != nullptr && desc.m_pDepthStencilView == pTextureView)
		{
			m_Framebuffers.Erase(pFramebuffer.first);
		}
	}
}


void BvFramebufferManagerGl::Destroy()
{
	for (auto&& pFramebuffer : m_Framebuffers)
	{
		delete pFramebuffer.second;
	}
	m_Framebuffers.Clear();
}
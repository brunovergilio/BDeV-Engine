#include "BvSwapChainVk.h"
#include "BvCommandQueueVk.h"
#include "BvUtilsVk.h"
#include "BvTextureVk.h"
#include "BvTypeConversionsVk.h"
#include "BDeV/Core/System/Window/BvWindow.h"
#include "BvFramebufferVk.h"
#include "BvCommandContextVk.h"
#include "BvRenderDeviceVk.h"
#include "BvRenderEngineVk.h"
#include "BvGPUFenceVk.h"
#include "BDeV/Core/System/Window/BvMonitor.h"


BvSwapChainVk::BvSwapChainVk(BvRenderDeviceVk* pDevice, VkSwapchainKHR swapChain, VkSurfaceKHR surface, const BvVector<VkImage>& images,
	const VkExtent2D& extents, BvWindow* pWindow, const SwapChainDesc& swapChainParams, BvCommandContextVk* pContext)
	: m_pWindow(pWindow), m_SwapChainDesc(swapChainParams), m_pDevice(pDevice), m_pCommandContext(pContext),
	m_pCommandQueue(static_cast<BvCommandContextVk*>(pContext)->GetCommandQueue()), m_Swapchain(swapChain), m_Surface(surface)
{
	m_pCommandContext->AddSwapChain(this);

	CreateTextureResources(images, extents);
	CreateSynchronizationResources();

	if (m_SwapChainDesc.m_WindowMode == SwapChainMode::kFullscreen)
	{
		SetTrueFullscreen(true);
	}

	m_IsReady = false;
	AcquireImage();
}


BvSwapChainVk::~BvSwapChainVk()
{
	Destroy();
}


void BvSwapChainVk::AcquireImage()
{
	if (!m_pWindow->IsVisible() || m_IsReady)
	{
		return;
	}

	auto& fenceData = m_Fences[m_CurrSemaphoreIndex];
	if (fenceData.m_pFence)
	{
		fenceData.m_pFence->Wait(fenceData.m_Value);
	}

	auto result = vkAcquireNextImageKHR(m_pDevice->GetHandle(), m_Swapchain, UINT64_MAX,
		m_ImageAcquiredSemaphores[m_CurrSemaphoreIndex], VK_NULL_HANDLE, &m_CurrImageIndex);
	if (result != VkResult::VK_SUCCESS)
	{
		if (result == VkResult::VK_SUBOPTIMAL_KHR || result == VkResult::VK_ERROR_OUT_OF_DATE_KHR)
		{
			Resize();
			return;
		}
		else
		{
			BV_ASSERT(result == VK_SUCCESS, "Failed to acquire a swap chain image");
		}
	}

	if (m_CurrImageIndex != m_CurrSemaphoreIndex)
	{
		std::swap(m_ImageAcquiredSemaphores[m_CurrImageIndex], m_ImageAcquiredSemaphores[m_CurrSemaphoreIndex]);
		std::swap(m_RenderCompleteSemaphores[m_CurrImageIndex], m_RenderCompleteSemaphores[m_CurrSemaphoreIndex]);
	}

	m_IsReady = true;
}


void BvSwapChainVk::Present(bool vSync)
{
	if (!m_pWindow->IsVisible() || !m_IsReady)
	{
		return;
	}
	m_IsReady = false;

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_Swapchain;
	presentInfo.pImageIndices = &m_CurrImageIndex;

	VkSemaphore semaphores[] = { GetCurrentRenderCompleteSemaphore() };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = semaphores;

	auto result = VK_SUCCESS;
	if (vSync == m_SwapChainDesc.m_VSync)
	{
		result = vkQueuePresentKHR(m_pCommandQueue->GetHandle(), &presentInfo);
	}
	else
	{
		m_SwapChainDesc.m_VSync = vSync;
		result = VkResult::VK_ERROR_OUT_OF_DATE_KHR;
	}

	if (result != VkResult::VK_SUCCESS)
	{
		if (result == VkResult::VK_SUBOPTIMAL_KHR || result == VkResult::VK_ERROR_OUT_OF_DATE_KHR)
		{
			Resize();
			return;
		}
		else
		{
			BV_ASSERT(result == VK_SUCCESS, "Failed to present swap chain image");
		}
	}

	m_CurrSemaphoreIndex = (m_CurrSemaphoreIndex + 1) % (u32)m_SwapChainTextures.Size();
}


void BvSwapChainVk::SetWindowMode(SwapChainMode mode, BvMonitor* pMonitor)
{
	auto pCurrMonitor = m_pWindow->GetWindowDesc().m_pMonitor;
	if (!pMonitor)
	{
		pMonitor = pCurrMonitor;
	}

	if (m_SwapChainDesc.m_WindowMode == mode)
	{
		if (mode == SwapChainMode::kWindowed || pMonitor == pCurrMonitor)
		{
			return;
		}
	}

	m_pCommandQueue->WaitIdle();
	SetTrueFullscreen(false);

	m_SwapChainDesc.m_WindowMode = mode;
	m_pWindow->SetFullscreen(mode != SwapChainMode::kWindowed, pMonitor);
	RecreateSwapChain();
}


void BvSwapChainVk::SetCurrentFence(BvGPUFenceVk* pFence, u64 value)
{
	m_Fences[m_CurrImageIndex] = FenceData{ pFence, value };
}


bool BvSwapChainVk::RecreateSwapChain()
{
	DestroyTextureResources();
	DestroySynchronizationResources();

	auto scObj = VkHelpers::CreateSwapChain(m_pDevice, m_pDevice->GetInstance(), m_pCommandContext->GetGroupIndex(), m_Swapchain,
		m_Surface, m_SwapChainDesc, m_pWindow);
	if (scObj.first != VK_SUCCESS)
	{
		return false;
	}

	m_Swapchain = scObj.second.m_SwapChain;
	CreateTextureResources(scObj.second.m_Images, scObj.second.m_Extents);
	CreateSynchronizationResources();

	if (m_SwapChainDesc.m_WindowMode == SwapChainMode::kFullscreen)
	{
		SetTrueFullscreen(true);
	}

	m_IsReady = false;
	AcquireImage();

	return true;
}


void BvSwapChainVk::Destroy()
{
	if (m_Swapchain)
	{
		m_pCommandQueue->WaitIdle();

		m_pCommandContext->RemoveSwapChain(this);

		SetTrueFullscreen(false);

		DestroySynchronizationResources();

		DestroyTextureResources();

		VkHelpers::DestroyDeviceObject(*m_pDevice, m_Swapchain);
		m_Swapchain = VK_NULL_HANDLE;
		VkHelpers::DestroyInstanceObject(m_pDevice->GetInstance(), m_Surface);
	}
}


void BvSwapChainVk::Resize()
{
	if (m_pWindow->IsClosed() || m_pWindow->IsMinimized())
	{
		return;
	}
	m_pCommandQueue->WaitIdle();

	SetTrueFullscreen(false);

	RecreateSwapChain();
}


void BvSwapChainVk::SetTrueFullscreen(bool value)
{
	auto device = m_pDevice->GetHandle();
	if (value)
	{
		if (!m_FullscreenAcquired)
		{
			if (vkAcquireFullScreenExclusiveModeEXT(device, m_Swapchain) == VK_SUCCESS)
			{
				m_FullscreenAcquired = true;
			}
			else
			{
				m_SwapChainDesc.m_WindowMode = SwapChainMode::kBorderlessFullscreen;
			}
		}
	}
	else
	{
		if (m_FullscreenAcquired)
		{
			vkReleaseFullScreenExclusiveModeEXT(device, m_Swapchain);
			m_FullscreenAcquired = false;
		}
	}
}

void BvSwapChainVk::CreateTextureResources(const BvVector<VkImage>& images, const VkExtent2D& extents)
{
	TextureDesc textureDesc;
	textureDesc.m_Size = { extents.width, extents.height, 1 };
	textureDesc.m_Format = m_SwapChainDesc.m_Format;
	textureDesc.m_UsageFlags = TextureUsage::kRenderTarget;

	TextureViewDesc textureViewDesc;
	textureViewDesc.m_Format = textureDesc.m_Format;

	m_SwapChainTextures.Reserve(images.Size());
	m_SwapChainTextureViews.Reserve(images.Size());
	for (auto i = 0u; i < images.Size(); i++)
	{
		m_SwapChainTextures.EmplaceBack(BV_RC_CREATE(BvTextureVk, m_pDevice, textureDesc, this, images[i]));

		textureViewDesc.m_pTexture = m_SwapChainTextures[i];
		auto viewObj = VkHelpers::CreateTextureView(m_pDevice, textureViewDesc);
		BV_ASSERT(viewObj.first == VK_SUCCESS, "Failed to create texture view for swap chain");
		m_SwapChainTextureViews.EmplaceBack(BV_RC_CREATE(BvTextureViewVk, m_pDevice, textureViewDesc, viewObj.second));
	}
}

void BvSwapChainVk::DestroyTextureResources()
{
	for (auto i = 0u; i < m_SwapChainTextures.Size(); i++)
	{
		m_pCommandContext->RemoveFramebuffers(m_SwapChainTextureViews[i]->GetHandle());
		m_SwapChainTextureViews[i]->Release();
		m_SwapChainTextures[i]->Release();
	}
	m_SwapChainTextureViews.Clear();
	m_SwapChainTextures.Clear();
}


void BvSwapChainVk::CreateSynchronizationResources()
{
	m_ImageAcquiredSemaphores.Reserve(m_SwapChainTextures.Size());
	m_RenderCompleteSemaphores.Reserve(m_SwapChainTextures.Size());
	for (auto i = 0u; i < m_SwapChainTextures.Size(); ++i)
	{
		auto semObj1 = VkHelpers::CreateSemaphore(m_pDevice, {}, false);
		auto semObj2 = VkHelpers::CreateSemaphore(m_pDevice, {}, false);

		BV_ASSERT(semObj1.first == VK_SUCCESS && semObj2.first == VK_SUCCESS, "Couldn't create swap chain semaphores");
		m_ImageAcquiredSemaphores.EmplaceBack(semObj1.second);
		m_RenderCompleteSemaphores.EmplaceBack(semObj2.second);
	}

	m_Fences.Resize(m_SwapChainTextures.Size());
}


void BvSwapChainVk::DestroySynchronizationResources()
{
	for (auto i = 0u; i < m_ImageAcquiredSemaphores.Size(); ++i)
	{
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_ImageAcquiredSemaphores[i]);
		VkHelpers::DestroyDeviceObject(*m_pDevice, m_RenderCompleteSemaphores[i]);
	}

	m_ImageAcquiredSemaphores.Clear();
	m_RenderCompleteSemaphores.Clear();
	m_Fences.Clear();
}
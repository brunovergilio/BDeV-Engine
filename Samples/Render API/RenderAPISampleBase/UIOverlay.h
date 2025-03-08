#pragma once


#include "BDeV/BDeV.h"
#include "Third Party/imgui.h"


class UIOverlay
{
public:
	UIOverlay();
	~UIOverlay();

	void Initialize(IBvRenderDevice* pDevice, IBvCommandContext* pContext, Format swapChainFormat, IBvRenderPass* pRenderPass);
	void Update(u32 w, u32 h);
	void Render(IBvCommandContext* pContext);
	void Shutdown();

private:
	void CreateVB(u64 size, u32 count);
	void CreateIB(u64 size, u32 count);

private:
	BvObjectHandle<IBvRenderDevice> m_Device;
	BvObjectHandle<IBvBuffer> m_VB;
	BvObjectHandle<IBvBuffer> m_IB;
	BvObjectHandle<IBvBufferView> m_VBView;
	BvObjectHandle<IBvBufferView> m_IBView;
	BvObjectHandle<IBvGraphicsPipelineState> m_Pipeline;
	BvObjectHandle<IBvShaderResourceLayout> m_SRL;
	BvObjectHandle<IBvTexture> m_Texture;
	BvObjectHandle<IBvTextureView> m_TextureView;
	BvObjectHandle<IBvSampler> m_Sampler;

	u32 m_VertexCount = 0;
	u32 m_IndexCount = 0;

	u32 m_Width = 0;
	u32 m_Height = 0;

	struct PushConstBlock
	{
		Float2 scale;
		Float2 translate;
	} m_PC;

	BvKeyboard m_Keyboard;
	BvMouse m_Mouse;
};
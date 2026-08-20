#pragma once


#include "BDeV/Core/BvCore.h"
#include "BDeV/Core/System/HID/BvKeyboard.h"
#include "BDeV/Core/System/HID/BvMouse.h"
#include "WolfUIOverlay.h"
#include "BDeV/Core/RenderAPI/BvRenderEngine.h"
#include "BDeV/Core/RenderAPI/BvTextureLoader.h"
#include "Shaders/2DPrimColor.h"
#include "Shaders/2DPrimTexture.h"
#include "../Utils/WolfCommon.h"


class WolfRenderer final
{
public:
	WolfRenderer() = default;
	~WolfRenderer() = default;

	void Initialize(BvWindow* pWindow);
	void Update(f32 deltaTime);
	void Render();
	void Shutdown();
	BvRCRef<IBvShader> CompileShader(const char* pSource, size_t length, ShaderStage stage);

	void CreateDeviceAndContext();
	void OnInitializeUI();
	void OnUpdateUI();
	void OnRenderUI();

	void BeginDrawDefaultUI();
	void EndDrawDefaultUI();

	void ToggleOverlay(bool enabled);

	BV_INLINE void NewColorBatch()
	{
		m_ColorBatches.EmplaceBack();
	}

	BV_INLINE void DrawLine(const BvVec2& from, const BvVec2& to, const BvVec4& color = BvVec4(1.0f, 1.0f, 1.0f, 1.0f))
	{
		auto& line = m_ColorBatches.Back().m_ColorLines.EmplaceBack();
		XMStoreFloat2(&line.m_FromPos, from);
		XMStoreFloat2(&line.m_ToPos, to);
		XMStoreFloat4(&line.m_Color, color);
	}

	BV_INLINE void DrawColoredSquare(const BvVec2& pos, const BvVec2& size, const BvVec4& color = BvVec4(1.0f, 1.0f, 1.0f, 1.0f))
	{
		auto& sq = m_ColorBatches.Back().m_ColorSquares.EmplaceBack();
		XMStoreFloat2(&sq.m_Pos, pos);
		XMStoreFloat2(&sq.m_Size, size);
		XMStoreFloat4(&sq.m_Color, color);
	}

	BV_INLINE void NewTextureBatch()
	{
		m_TextureBatches.EmplaceBack();
	}

	BV_INLINE void DrawLine(const BvVec2& from, const BvVec2& to, const BvVec2& uvFrom, const BvVec2& uvTo, u32 textureIndex, const BvVec4& colorMultiplier = BvVec4(1.0f, 1.0f, 1.0f))
	{
		auto& line = m_TextureBatches.Back().m_Lines.EmplaceBack();
		XMStoreFloat2(&line.m_FromPos, from);
		XMStoreFloat2(&line.m_ToPos, to);
		XMStoreFloat2(&line.m_UVs[0], uvFrom);
		XMStoreFloat2(&line.m_UVs[1], uvTo);
		XMStoreFloat4(&line.m_ColorMultiplier, colorMultiplier);
		line.m_TextureIndex = textureIndex;
	}

	BV_INLINE void DrawSquare(const BvVec2& pos, const BvVec2& size, u32 textuereIndex, const BvVec4& colorMultiplier = BvVec4(1.0f))
	{
		auto& sq = m_TextureBatches.Back().m_Squares.EmplaceBack();
		sq.m_Pos = pos.ToFloat();
		sq.m_Size = size.ToFloat();
		sq.m_UVs[0] = Float2(0.0f, 0.0f);
		sq.m_UVs[1] = Float2(1.0f, 1.0f);
		sq.m_ColorMultiplier = colorMultiplier.ToFloat();
		sq.m_TextureIndex = textuereIndex;
	}

	BV_INLINE void DrawSquare(const BvVec2& pos, const BvVec2& size, const BvVec2& uvStart, const BvVec2& uvEnd, u32 textuereIndex, const BvVec4& colorMultiplier = BvVec4(1.0f))
	{
		auto& sq = m_TextureBatches.Back().m_Squares.EmplaceBack();
		XMStoreFloat2(&sq.m_Pos, pos);
		XMStoreFloat2(&sq.m_Size, size);
		XMStoreFloat2(&sq.m_UVs[0], uvStart);
		XMStoreFloat2(&sq.m_UVs[1], uvEnd);
		XMStoreFloat4(&sq.m_ColorMultiplier, colorMultiplier);
		sq.m_TextureIndex = textuereIndex;
	}

	BV_INLINE const BvWindow* GetWindow() const { return m_pWindow; }

	u32 CreateTexture(const WolfImage& image);
	u32 CreateTexture(const u8* pData, u32 size);

private:
	void CreatePipelineResources();
	void CreateTextures();
	void DrawColorPrimitives();
	void DrawTexturePrimitives();

private:
	struct ColorBatch
	{
		struct ColorLine
		{
			Float4 m_Color;
			Float2 m_FromPos;
			Float2 m_ToPos;
		};

		struct ColorSquare
		{
			Float4 m_Color;
			Float2 m_Pos;
			Float2 m_Size;
		};

		BvVector<ColorSquare> m_ColorSquares;
		BvVector<ColorLine> m_ColorLines;
	};

	struct TextureBatch
	{
		struct TextureLine
		{
			Float2 m_FromPos;
			Float2 m_ToPos;
			Float2 m_UVs[2];
			Float4 m_ColorMultiplier;
			u32 m_TextureIndex;
		};

		struct TextureSquare
		{
			Float2 m_Pos;
			Float2 m_Size;
			Float2 m_UVs[2];
			Float4 m_ColorMultiplier;
			u32 m_TextureIndex;
		};

		BvVector<TextureSquare> m_Squares;
		BvVector<TextureLine> m_Lines;
	};

	BvVector<ColorBatch> m_ColorBatches;
	BvVector<TextureBatch> m_TextureBatches;

	PrimColor::Constants m_PrimColorConstants;
	PrimTexture::Constants m_PrimTextureConstants;

	BvSharedLib m_RenderLib;
	BvSharedLib m_ToolsLib;
	RenderDeviceDesc m_RenderDeviceDesc;
	BvWindow* m_pWindow = nullptr;
	BvRCRef<IBvRenderEngine> m_pEngine;
	BvRCRef<IBvRenderDevice> m_Device;
	BvRCRef<IBvCommandContext> m_Context;
	BvRCRef<IBvSwapChain> m_SwapChain;
	BvRCRef<IBvShaderCompiler> m_ShaderCompiler;
	BvRCRef<IBvTextureLoader> m_TextureLoader;

	BvRCRef<IBvShaderResourceLayout> m_SRLColorPrim;
	BvRCRef<IBvGraphicsPipelineState> m_PSOColorSquare;
	BvRCRef<IBvGraphicsPipelineState> m_PSOColorLine;

	BvRCRef<IBvShaderResourceLayout> m_SRLTexturePrim;
	BvRCRef<IBvGraphicsPipelineState> m_PSOTextureSquare;
	BvRCRef<IBvGraphicsPipelineState> m_PSOTextureLine;

	BvRCRef<IBvBuffer> m_SquaresVB;
	BvRCRef<IBvBufferView> m_SquaresVBV;

	BvRCRef<IBvBuffer> m_LinesVB;
	BvRCRef<IBvBufferView> m_LinesVBV;

	struct TextureData
	{
		BvRCRef<IBvTexture> m_Texture;
		BvRCRef<IBvTextureView> m_TextureView;
	};
	BvVector<TextureData> m_Textures;
	BvRCRef<IBvSampler> m_PointSampler;

	WolfUIOverlay m_Overlay;
	bool m_UseOverlay = true;
};
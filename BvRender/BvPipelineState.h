#pragma once


#include "BvRender/BvShaderResource.h"
#include "BvRender/BvRenderPass.h"
#include "BvCore/Utils/BvUtils.h"
#include "BvCore/Container/BvFixedVector.h"
#include "BvCore/Container/BvString.h"


constexpr u32 kMaxShaderStages = 5;
constexpr u32 kMaxVertexAttributes = 16;
constexpr u32 kMaxVertexBindings = 16;


enum class InputRate : u8
{
	kPerVertex,
	kPerInstance,
};


enum class Topology : u8
{
	kUndefined,
	kPointList,
	kLineList,
	kLineStrip,
	kTriangleList,
	kTriangleStrip,
	kLineListAdj,
	kLineStripAdj,
	kTriangleListAdj,
	kTriangleStripAdj,
};


enum class FillMode : u8
{
	kSolid,
	kWireframe,
};


enum class CullMode : u8
{
	kNone,
	kFront,
	kBack,
	kFrontAndBack,
};


enum class FrontFace : u8
{
	kClockwise,
	kCounterClockwise,
};


enum class StencilOp : u8
{
	kKeep,
	kZero,
	kReplace,
	kIncrSat,
	kDecrSat,
	kInvert,
	kIncrWrap,
	kDecrWrap,
};


enum class BlendFactor : u8
{
	kZero,
	kOne,
	kSrcColor,
	kInvSrcColor,
	kSrcAlpha,
	kInvkSrcAlpha,
	kDstAlpha,
	kInvDstAlpha,
	kDstColor,
	kInvDstColor,
	kSrcAlphaSat,
	kBlendFactor,
	kInvBlendFactor,
	kSrc1Color,
	kInvSrc1Color,
	kSrc1Alpha,
	kInvkSrc1Alpha,
};


enum class BlendOp : u8
{
	kAdd,
	kSubtract,
	kRevSubtract,
	kMin,
	kMax
};


enum class LogicOp : u8
{
	kClear,
	kSet,
	kCopy,
	kCopyInverted,
	kNoOp,
	kInvert,
	kAnd,
	kNand,
	kOr,
	kNor,
	kXor,
	kEquiv,
	kAndReverse,
	kAndInverted,
	kOrReverse,
	kOrInverted
};


struct ShaderByteCodeDesc
{
	const u8 * m_pByteCode = nullptr;
	size_t m_ByteCodeSize = 0;
	BvStringT m_EntryPoint{};
	ShaderStage m_ShaderStage = ShaderStage::kUnknown;
};


struct VertexAttributeDesc
{
	u32		m_Location = 0;
	u32		m_Binding = 0;
	u32		m_Offset = 0;
	Format	m_Format = Format::kUndefined;
};


struct VertexBindingDesc
{
	u32			m_Binding = 0;
	u32			m_Stride = 0;
	u32			m_InstanceRate = 0;
	InputRate	m_InputRate = InputRate::kPerVertex;
};


struct InputAssemblyStateDesc
{
	Topology	m_Topology = Topology::kTriangleList;
	bool		m_PrimitiveRestart = false;
};


struct TessellationStateDesc
{
	u8 m_PatchControlPoints = 0;
};


struct ViewportStateDesc
{
	u8 m_MaxViewportCount = 1;
	u8 m_MaxScissorCount = 1;
};


struct RasterizerStateDesc
{
	FillMode m_FillMode = FillMode::kSolid;
	CullMode m_CullMode = CullMode::kNone;
	FrontFace m_FrontFace = FrontFace::kClockwise;
	bool m_EnableDepthBias = false;
	float m_DepthBias = 0.0f;
	float m_DepthBiasClamp = 0.0f;
	float m_DepthBiasSlope = 0.0f;
	bool m_EnableDepthClip = false;
	bool m_EnableConservativeRasterization = false;
};


struct StencilDesc
{
	StencilOp m_StencilFailOp = StencilOp::kKeep;
	StencilOp m_StencilDepthFailOp = StencilOp::kKeep;
	StencilOp m_StencilPassOp = StencilOp::kKeep;
	CompareOp m_StencilFunc = CompareOp::kNever;
};


struct DepthStencilDesc
{
	bool m_DepthTestEnable = false;
	bool m_DepthWriteEnable = false;
	CompareOp m_DepthOp = CompareOp::kNever;
	bool m_StencilTestEnable = false;
	u8 m_StencilReadMask = 0;
	u8 m_StencilWriteMask = 0;
	bool m_DepthBoundsTestEnable = false;
	StencilDesc m_StencilFront{};
	StencilDesc m_StencilBack{};
};


struct MultisampleStateDesc
{
	u8 m_SampleCount = 1;
};


struct BlendAttachmentStateDesc
{
	bool m_BlendEnable = false;
	BlendFactor m_SrcBlend = BlendFactor::kZero;
	BlendFactor m_DstBlend = BlendFactor::kZero;
	BlendOp m_BlendOp = BlendOp::kAdd;
	BlendFactor m_SrcBlendAlpha = BlendFactor::kZero;
	BlendFactor m_DstBlendAlpha = BlendFactor::kZero;
	BlendOp m_BlendOpAlpha = BlendOp::kAdd;
	u8 m_RenderTargetWriteMask = 0xF;
};


struct BlendStateDesc
{
	BvFixedVector<BlendAttachmentStateDesc, kMaxRenderTargets> m_BlendAttachments;
	LogicOp m_LogicOp = LogicOp::kClear;
	bool m_LogicEnable = false;
	bool m_AlphaToCoverageEnable = false;
};


struct GraphicsPipelineStateDesc
{
	BvFixedVector<VertexBindingDesc, kMaxVertexBindings>		m_VertexBindingDesc;
	BvFixedVector<VertexAttributeDesc, kMaxVertexAttributes>	m_VertexAttributeDesc;
	BvFixedVector<ShaderByteCodeDesc, kMaxShaderStages>			m_ShaderStages;
	InputAssemblyStateDesc										m_InputAssemblyStateDesc;
	TessellationStateDesc										m_TessellationStateDesc;
	ViewportStateDesc											m_ViewportStateDesc;
	RasterizerStateDesc											m_RasterizerStateDesc;
	DepthStencilDesc											m_DepthStencilDesc;
	MultisampleStateDesc										m_MultisampleStateDesc;
	BlendStateDesc												m_BlendStateDesc;
	BvShaderResourceLayout *									m_pShaderResourceLayout = nullptr;
	BvRenderPass *												m_pRenderPass = nullptr;
};


struct ComputePipelineStateDesc 
{
	ShaderByteCodeDesc			m_ShaderByteCodeDesc;
	BvShaderResourceLayout *	m_pShaderResourceLayout = nullptr;
};


class BvGraphicsPipelineState
{
	BV_NOCOPYMOVE(BvGraphicsPipelineState);

public:
	BV_INLINE const GraphicsPipelineStateDesc & GetDesc() const { return m_PipelineStateDesc; }

protected:
	BvGraphicsPipelineState(const GraphicsPipelineStateDesc & pipelineStateDesc)
		: m_PipelineStateDesc(pipelineStateDesc) {}
	virtual ~BvGraphicsPipelineState() = 0 {}

protected:
	GraphicsPipelineStateDesc m_PipelineStateDesc;
};


class BvComputePipelineState
{
	BV_NOCOPYMOVE(BvComputePipelineState);

public:
	BV_INLINE const ComputePipelineStateDesc & GetDesc() const { return m_PipelineStateDesc; }

protected:
	BvComputePipelineState(const ComputePipelineStateDesc & pipelineStateDesc)
		: m_PipelineStateDesc(pipelineStateDesc) {}
	virtual ~BvComputePipelineState() = 0 {}

protected:
	ComputePipelineStateDesc m_PipelineStateDesc;
};
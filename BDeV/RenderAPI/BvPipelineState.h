#pragma once


#include "BvShaderResource.h"
#include "BvRenderPass.h"
#include "BDeV/Utils/BvUtils.h"
#include "BDeV/Container/BvFixedVector.h"
#include "BDeV/Container/BvString.h"


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
	kDstColor,
	kInvDstColor,
	kSrcAlpha,
	kInvkSrcAlpha,
	kDstAlpha,
	kInvDstAlpha,
	kBlendFactor,
	kInvBlendFactor,
	kAlphaFactor,
	kInvAlphaFactor,
	kSrcAlphaSat,
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


enum class ColorMask : u8
{
	kNone = 0,
	kRed = BvBit(0),
	kGreen = BvBit(1),
	kBlue = BvBit(2),
	kAlpha = BvBit(3),
	kAll = kRed | kGreen | kBlue | kAlpha
};


struct ShaderByteCodeDesc
{
	const u8 * m_pByteCode = nullptr;
	size_t m_ByteCodeSize = 0;
	BvString m_EntryPoint{};
	ShaderStage m_ShaderStage = ShaderStage::kUnknown;
};


struct VertexInputDesc
{
	// Index of the input attribute (In Vulkan and OpenGL, this maps to the location index that an input variable is.
	// In D3D this is only used if there are multiple elements with the same semantic.
	u32			m_Location = 0;
	// Buffer slot this attribute refers to
	u32			m_Binding = 0;
	// The stride of the variable or struct used to represent this element
	u32			m_Stride = 0;
	// The offset of this variable in the struct
	u32			m_Offset = 0;
	// The instance rate / step, which determines how many instances will be drawn with the same per-instance
	// data before moving to the next one - this should be 0 if the input rate is InputRate::kPerVertex
	u32			m_InstanceRate = 0;
	// The input rate, can be per vertex or per instance
	InputRate	m_InputRate = InputRate::kPerVertex;
	// Variable format
	Format		m_Format = Format::kUnknown;
	// Determines whether the variable data is normalized ([-1, 1] for signed types, [0, 1] for unsigned types)
	bool		m_Normalized = false;

	bool operator==(const VertexInputDesc& rhs) const
	{
		return m_Location == rhs.m_Location && m_Binding == rhs.m_Binding && m_Stride == rhs.m_Stride
			&& m_Offset == rhs.m_Offset && m_InputRate == rhs.m_InputRate && m_InstanceRate == rhs.m_InstanceRate
			&& m_Format == rhs.m_Format && m_Normalized == rhs.m_Normalized;
	}
};


struct InputAssemblyStateDesc
{
	Topology	m_Topology = Topology::kTriangleList;
	bool		m_PrimitiveRestart = false;
	IndexFormat m_IndexFormatForPrimitiveRestart = IndexFormat::kU16;
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
	BlendOp m_AlphaBlendOp = BlendOp::kAdd;
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
	BvFixedVector<VertexInputDesc, kMaxVertexAttributes>		m_VertexInputDesc;
	BvFixedVector<ShaderByteCodeDesc, kMaxShaderStages>			m_ShaderStages;
	InputAssemblyStateDesc										m_InputAssemblyStateDesc;
	TessellationStateDesc										m_TessellationStateDesc;
	ViewportStateDesc											m_ViewportStateDesc;
	RasterizerStateDesc											m_RasterizerStateDesc;
	DepthStencilDesc											m_DepthStencilDesc;
	MultisampleStateDesc										m_MultisampleStateDesc;
	BlendStateDesc												m_BlendStateDesc;
	BvShaderResourceLayout*										m_pShaderResourceLayout = nullptr;
	BvRenderPass*												m_pRenderPass = nullptr;
	BvFixedVector<Format, kMaxRenderTargets>					m_RenderTargetFormats;
	Format														m_DepthStencilFormat = Format::kUnknown;
	u32															m_SampleMask = kMax<u32>;
};


struct ComputePipelineStateDesc
{
	ShaderByteCodeDesc			m_ShaderByteCodeDesc;
	BvShaderResourceLayout*		m_pShaderResourceLayout = nullptr;
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
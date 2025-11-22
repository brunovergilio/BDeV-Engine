#include "BvTypeConversionsD3D12.h"
#include "BvTextureD3D12.h"
#include "BvBufferD3D12.h"
#include <bit>


D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(BufferUsage usage)
{
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
	if (!EHasFlag(usage, BufferUsage::kStructuredBuffer))
	{
		flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	}
	if (EHasFlag(usage, BufferUsage::kRWStructuredBuffer))
	{
		flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	if (EHasFlag(usage, BufferUsage::kRayTracing))
	{
		flags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	return flags;
}


D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(TextureUsage usage)
{
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
	if (EHasFlag(usage, TextureUsage::kRenderTarget))
	{
		flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	if (EHasFlag(usage, TextureUsage::kDepthStencilTarget))
	{
		flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	if (!(EHasFlag(usage, TextureUsage::kShaderResource) || EHasFlag(usage, TextureUsage::kInputAttachment)))
	{
		flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	}
	if (EHasFlag(usage, TextureUsage::kUnorderedAccess))
	{
		flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	return flags;
}


CD3DX12_RESOURCE_DESC GetD3D12ResourceDesc(const TextureDesc& textureDesc)
{
	switch (textureDesc.m_ImageType)
	{
	case TextureType::kTexture1D:
		return CD3DX12_RESOURCE_DESC::Tex1D(DXGI_FORMAT(textureDesc.m_Format), textureDesc.m_Size.width, textureDesc.m_ArraySize, textureDesc.m_MipLevels,
			GetD3D12ResourceFlags(textureDesc.m_UsageFlags));
	case TextureType::kTexture2D:
		return CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT(textureDesc.m_Format), textureDesc.m_Size.width, textureDesc.m_Size.height, textureDesc.m_ArraySize,
			textureDesc.m_MipLevels, textureDesc.m_SampleCount, 0, GetD3D12ResourceFlags(textureDesc.m_UsageFlags));
	case TextureType::kTexture3D:
		return CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT(textureDesc.m_Format), textureDesc.m_Size.width, textureDesc.m_Size.height, textureDesc.m_Size.depth,
			textureDesc.m_MipLevels, GetD3D12ResourceFlags(textureDesc.m_UsageFlags));
	default:
		return CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT(textureDesc.m_Format), textureDesc.m_Size.width, textureDesc.m_Size.height, textureDesc.m_ArraySize,
			textureDesc.m_MipLevels, textureDesc.m_SampleCount, 0, GetD3D12ResourceFlags(textureDesc.m_UsageFlags));
	}
}


D3D12_HEAP_TYPE GetD3D12HeapType(MemoryType memoryType)
{
	switch (memoryType)
	{
	case MemoryType::kDevice: return D3D12_HEAP_TYPE_DEFAULT;
	case MemoryType::kUpload:
	case MemoryType::kUploadNC: return D3D12_HEAP_TYPE_UPLOAD;
	case MemoryType::kReadBack:
	case MemoryType::kReadBackNC: return D3D12_HEAP_TYPE_READBACK;
	case MemoryType::kShared:
	default:
		BV_ASSERT(false, "Memory type not supported");
		return D3D12_HEAP_TYPE_DEFAULT;
	}
}


D3D12_RESOURCE_STATES GetD3D12ResourceState(ResourceState resourceState)
{
	switch (resourceState)
	{
	case ResourceState::kCommon:				return D3D12_RESOURCE_STATE_COMMON;
	case ResourceState::kVertexBuffer:			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	case ResourceState::kIndexBuffer:			return D3D12_RESOURCE_STATE_INDEX_BUFFER;
	case ResourceState::kIndirectBuffer:		return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
	case ResourceState::kConstantBuffer:		return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	case ResourceState::kShaderResource:		return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
	case ResourceState::kPixelShaderResource:	return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	case ResourceState::kRWResource:			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case ResourceState::kTransferSrc:			return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case ResourceState::kTransferDst:			return D3D12_RESOURCE_STATE_COPY_DEST;
	case ResourceState::kRenderTarget:			return D3D12_RESOURCE_STATE_RENDER_TARGET;
	case ResourceState::kDepthStencilRead:		return D3D12_RESOURCE_STATE_DEPTH_READ;
	case ResourceState::kDepthStencilWrite:		return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case ResourceState::kPresent:				return D3D12_RESOURCE_STATE_PRESENT;
	case ResourceState::kResolveSrc:			return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
	case ResourceState::kResolveDst:			return D3D12_RESOURCE_STATE_RESOLVE_DEST;
	case ResourceState::kPredication:			return D3D12_RESOURCE_STATE_PREDICATION;
	case ResourceState::kShadingRate:			return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
	case ResourceState::kASBuildRead:
	case ResourceState::kASBuildWrite:			return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
	case ResourceState::kASPostBuildBuffer:		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	default:									return D3D12_RESOURCE_STATE_COMMON;
	}
}


D3D12_DESCRIPTOR_RANGE_TYPE GetD3D12DescriptorRangeType(ShaderResourceType type)
{
	switch (type)
	{
	case ShaderResourceType::kConstantBuffer: return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	case ShaderResourceType::kStructuredBuffer: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	case ShaderResourceType::kRWStructuredBuffer: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	case ShaderResourceType::kFormattedBuffer: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	case ShaderResourceType::kRWFormattedBuffer: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	case ShaderResourceType::kTexture: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	case ShaderResourceType::kRWTexture: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	case ShaderResourceType::kSampler: return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	case ShaderResourceType::kInputAttachment: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	default:
		BV_ASSERT(false, "Should never get here");
	}
	return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
}


D3D12_SHADER_VISIBILITY GetD3D12ShaderVisibility(ShaderStage shaderStages)
{
	if (std::popcount((u32)shaderStages) > 1)
	{
		return D3D12_SHADER_VISIBILITY_ALL;
	}

	switch (shaderStages)
	{
	case ShaderStage::kVertex: return D3D12_SHADER_VISIBILITY_VERTEX;
	case ShaderStage::kHullOrControl: return D3D12_SHADER_VISIBILITY_HULL;
	case ShaderStage::kDomainOrEvaluation: return D3D12_SHADER_VISIBILITY_DOMAIN;
	case ShaderStage::kGeometry: return D3D12_SHADER_VISIBILITY_GEOMETRY;
	case ShaderStage::kPixelOrFragment: return D3D12_SHADER_VISIBILITY_PIXEL;
	case ShaderStage::kMesh: return D3D12_SHADER_VISIBILITY_MESH;
	case ShaderStage::kAmplificationOrTask: return D3D12_SHADER_VISIBILITY_AMPLIFICATION;
	default:
		return D3D12_SHADER_VISIBILITY_ALL;
	}
}


CD3DX12_STATIC_SAMPLER_DESC GetD3D12StaticSamplerDesc(const SamplerDesc& samplerDesc, u32 binding, u32 set, D3D12_SHADER_VISIBILITY shaderVisibility)
{
	return CD3DX12_STATIC_SAMPLER_DESC(binding, GetD3D12Filter(samplerDesc), GetD3D12TextureAddressMode(samplerDesc.m_AddressModeU),
		GetD3D12TextureAddressMode(samplerDesc.m_AddressModeV), GetD3D12TextureAddressMode(samplerDesc.m_AddressModeW), samplerDesc.m_MipLodBias,
		samplerDesc.m_MaxAnisotropy, GetD3D12ComparisonFunc(samplerDesc.m_CompareOp),
		GetD3D12StaticBorderColor(samplerDesc), samplerDesc.m_MinLod, samplerDesc.m_MaxLod, shaderVisibility, set);
}


D3D12_FILTER GetD3D12Filter(const SamplerDesc& samplerDesc)
{
	if (samplerDesc.m_AnisotropyEnable)
	{
		if (samplerDesc.m_CompareOp != CompareOp::kNone)
		{
			return D3D12_FILTER_COMPARISON_ANISOTROPIC;
		}
		else if (samplerDesc.m_ReductionMode != ReductionMode::kStandard)
		{
			return samplerDesc.m_ReductionMode == ReductionMode::kMin ? D3D12_FILTER_MINIMUM_ANISOTROPIC : D3D12_FILTER_MAXIMUM_ANISOTROPIC;
		}
		else
		{
			return D3D12_FILTER_ANISOTROPIC;
		}
	}

	switch (samplerDesc.m_MinFilter)
	{
	case Filter::kPoint:
		switch (samplerDesc.m_MagFilter)
		{
		case Filter::kPoint:
			switch (samplerDesc.m_MipmapMode)
			{
			case MipMapFilter::kPoint:
			{
				if (samplerDesc.m_CompareOp != CompareOp::kNone)
				{
					return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
				}
				else if (samplerDesc.m_ReductionMode != ReductionMode::kStandard)
				{
					return samplerDesc.m_ReductionMode == ReductionMode::kMin ? D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT : D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
				}
				else
				{
					return D3D12_FILTER_MIN_MAG_MIP_POINT;
				}
			}
			case MipMapFilter::kLinear:
			{
				if (samplerDesc.m_CompareOp != CompareOp::kNone)
				{
					return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
				}
				else if (samplerDesc.m_ReductionMode != ReductionMode::kStandard)
				{
					return samplerDesc.m_ReductionMode == ReductionMode::kMin ? D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR : D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
				}
				else
				{
					return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
				}
			}
			}
		case Filter::kLinear:
			switch (samplerDesc.m_MipmapMode)
			{
			case MipMapFilter::kPoint:
			{
				if (samplerDesc.m_CompareOp != CompareOp::kNone)
				{
					return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
				}
				else if (samplerDesc.m_ReductionMode != ReductionMode::kStandard)
				{
					return samplerDesc.m_ReductionMode == ReductionMode::kMin ? D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT : D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
				}
				else
				{
					return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
				}
			}
			case MipMapFilter::kLinear:
			{
				if (samplerDesc.m_CompareOp != CompareOp::kNone)
				{
					return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
				}
				else if (samplerDesc.m_ReductionMode != ReductionMode::kStandard)
				{
					return samplerDesc.m_ReductionMode == ReductionMode::kMin ? D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR : D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
				}
				else
				{
					return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
				}
			}
			}
		}
	case Filter::kLinear:
		switch (samplerDesc.m_MagFilter)
		{
		case Filter::kPoint:
			switch (samplerDesc.m_MipmapMode)
			{
			case MipMapFilter::kPoint:
			{
				if (samplerDesc.m_CompareOp != CompareOp::kNone)
				{
					return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
				}
				else if (samplerDesc.m_ReductionMode != ReductionMode::kStandard)
				{
					return samplerDesc.m_ReductionMode == ReductionMode::kMin ? D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT : D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
				}
				else
				{
					return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
				}
			}
			case MipMapFilter::kLinear:
			{
				if (samplerDesc.m_CompareOp != CompareOp::kNone)
				{
					return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				}
				else if (samplerDesc.m_ReductionMode != ReductionMode::kStandard)
				{
					return samplerDesc.m_ReductionMode == ReductionMode::kMin ? D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR : D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				}
				else
				{
					return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				}
			}
			}
		case Filter::kLinear:
			switch (samplerDesc.m_MipmapMode)
			{
			case MipMapFilter::kPoint:
			{
				if (samplerDesc.m_CompareOp != CompareOp::kNone)
				{
					return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
				}
				else if (samplerDesc.m_ReductionMode != ReductionMode::kStandard)
				{
					return samplerDesc.m_ReductionMode == ReductionMode::kMin ? D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT : D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
				}
				else
				{
					return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				}
			}
			case MipMapFilter::kLinear:
			{
				if (samplerDesc.m_CompareOp != CompareOp::kNone)
				{
					return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
				}
				else if (samplerDesc.m_ReductionMode != ReductionMode::kStandard)
				{
					return samplerDesc.m_ReductionMode == ReductionMode::kMin ? D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
				}
				else
				{
					return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				}
			}
			}
		}
	}

	return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
}


D3D12_TEXTURE_ADDRESS_MODE GetD3D12TextureAddressMode(AddressMode addressMode)
{
	switch (addressMode)
	{
	case AddressMode::kWrap: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case AddressMode::kMirror: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	case AddressMode::kClamp: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case AddressMode::kBorder: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	case AddressMode::kMirrorOnce: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
	default: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	}
}


D3D12_COMPARISON_FUNC GetD3D12ComparisonFunc(CompareOp compareOp)
{
	switch (compareOp)
	{
	case CompareOp::kNever: return D3D12_COMPARISON_FUNC_NEVER;
	case CompareOp::kLess: return D3D12_COMPARISON_FUNC_LESS;
	case CompareOp::kEqual: return D3D12_COMPARISON_FUNC_EQUAL;
	case CompareOp::kLessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case CompareOp::kGreater: return D3D12_COMPARISON_FUNC_GREATER;
	case CompareOp::kNotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case CompareOp::kGreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case CompareOp::kAlways: return D3D12_COMPARISON_FUNC_ALWAYS;
	default: return D3D12_COMPARISON_FUNC_NONE;
	}
}


D3D12_STATIC_BORDER_COLOR GetD3D12StaticBorderColor(const SamplerDesc& samplerDesc)
{
	if (samplerDesc.m_BorderColor[0] == 0.0f
		|| samplerDesc.m_BorderColor[1] == 0.0f
		|| samplerDesc.m_BorderColor[2] == 0.0f
		|| samplerDesc.m_BorderColor[3] == 1.0f)
	{
		return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	}
	else if (samplerDesc.m_BorderColor[0] == 1.0f
		|| samplerDesc.m_BorderColor[1] == 1.0f
		|| samplerDesc.m_BorderColor[2] == 1.0f
		|| samplerDesc.m_BorderColor[3] == 1.0f)
	{
		return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	}

	return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
}


D3D12_SAMPLER_DESC GetD3D12SamplerDesc(const SamplerDesc& samplerDesc)
{
	return D3D12_SAMPLER_DESC{ GetD3D12Filter(samplerDesc), GetD3D12TextureAddressMode(samplerDesc.m_AddressModeU), GetD3D12TextureAddressMode(samplerDesc.m_AddressModeV),
		GetD3D12TextureAddressMode(samplerDesc.m_AddressModeW), samplerDesc.m_MipLodBias, samplerDesc.m_AnisotropyEnable ? 0 : UINT(samplerDesc.m_MaxAnisotropy),
		GetD3D12ComparisonFunc(samplerDesc.m_CompareOp), { samplerDesc.m_BorderColor[0], samplerDesc.m_BorderColor[1], samplerDesc.m_BorderColor[2], samplerDesc.m_BorderColor[3], },
		samplerDesc.m_MinLod, samplerDesc.m_MaxLod };
}


D3D12_ROOT_SIGNATURE_FLAGS GetD3D12RootSignatureFlags(ShaderStage shaderStages)
{
	D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	if (!EHasFlag(shaderStages, ShaderStage::kVertex)) { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS; }
	if (!EHasFlag(shaderStages, ShaderStage::kHullOrControl)) {	flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS; }
	if (!EHasFlag(shaderStages, ShaderStage::kDomainOrEvaluation)) { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS; }
	if (!EHasFlag(shaderStages, ShaderStage::kGeometry)) { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS; }
	if (!EHasFlag(shaderStages, ShaderStage::kPixelOrFragment)) { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS; }
	if (!EHasFlag(shaderStages, ShaderStage::kMesh)) { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS; }
	if (!EHasFlag(shaderStages, ShaderStage::kAmplificationOrTask)) { flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS; }

	return flags;
}


D3D12_INPUT_CLASSIFICATION GetD3D12InputClassification(InputRate inputRate)
{
	return inputRate == InputRate::kPerVertex ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
}


constexpr D3D12_BLEND kBlends[] =
{
	D3D12_BLEND_ZERO, 	// kZero,
	D3D12_BLEND_ONE, 	// kOne,
	D3D12_BLEND_SRC_COLOR, 	// kSrcColor,
	D3D12_BLEND_INV_SRC_COLOR, 	// kInvSrcColor,
	D3D12_BLEND_DEST_COLOR, 	// kDstColor,
	D3D12_BLEND_INV_DEST_COLOR, 	// kInvDstColor,
	D3D12_BLEND_SRC_ALPHA, 	// kSrcAlpha,
	D3D12_BLEND_INV_SRC_ALPHA, 	// kInvkSrcAlpha,
	D3D12_BLEND_DEST_ALPHA, 	// kDstAlpha,
	D3D12_BLEND_INV_DEST_ALPHA, 	// kInvDstAlpha,
	D3D12_BLEND_BLEND_FACTOR, 	// kBlendFactor,
	D3D12_BLEND_INV_BLEND_FACTOR, 	// kInvBlendFactor,
	D3D12_BLEND_ALPHA_FACTOR, 	// kAlphaFactor,
	D3D12_BLEND_INV_ALPHA_FACTOR, 	// kInvAlphaFactor,
	D3D12_BLEND_SRC_ALPHA_SAT, 	// kSrcAlphaSat,
	D3D12_BLEND_SRC1_COLOR, 	// kSrc1Color,
	D3D12_BLEND_INV_SRC1_COLOR, 	// kInvSrc1Color,
	D3D12_BLEND_SRC1_ALPHA, 	// kSrc1Alpha,
	D3D12_BLEND_INV_SRC1_ALPHA, 	// kInvkSrc1Alpha,
};


D3D12_BLEND GetD3D12Blend(BlendFactor blendFactor)
{
	return kBlends[u32(blendFactor)];
}


constexpr D3D12_BLEND_OP kBlendOps[] =
{
	D3D12_BLEND_OP_ADD,
	D3D12_BLEND_OP_SUBTRACT,
	D3D12_BLEND_OP_REV_SUBTRACT,
	D3D12_BLEND_OP_MIN,
	D3D12_BLEND_OP_MAX,
};


D3D12_BLEND_OP GetD3D12BlendOp(BlendOp blendOp)
{
	return kBlendOps[u32(blendOp)];
}


constexpr D3D12_LOGIC_OP kLogicOps[] =
{
	D3D12_LOGIC_OP_CLEAR, //kClear,
	D3D12_LOGIC_OP_SET, //kSet,
	D3D12_LOGIC_OP_COPY, //kCopy,
	D3D12_LOGIC_OP_COPY_INVERTED, //kCopyInverted,
	D3D12_LOGIC_OP_NOOP, //kNoOp,
	D3D12_LOGIC_OP_INVERT, //kInvert,
	D3D12_LOGIC_OP_AND, //kAnd,
	D3D12_LOGIC_OP_NAND, //kNand,
	D3D12_LOGIC_OP_OR, //kOr,
	D3D12_LOGIC_OP_NOR, //kNor,
	D3D12_LOGIC_OP_XOR, //kXor,
	D3D12_LOGIC_OP_EQUIV, //kEquiv,
	D3D12_LOGIC_OP_AND_REVERSE, //kAndReverse,
	D3D12_LOGIC_OP_AND_INVERTED, //kAndInverted,
	D3D12_LOGIC_OP_OR_REVERSE, //kOrReverse,
	D3D12_LOGIC_OP_OR_INVERTED, //kOrInverted
};


D3D12_LOGIC_OP GetD3D12LogicOp(LogicOp logicOp)
{
	return kLogicOps[u32(logicOp)];
}


constexpr D3D12_STENCIL_OP kStencilOps[] =
{
	D3D12_STENCIL_OP_KEEP, // kKeep
	D3D12_STENCIL_OP_ZERO, // kZero
	D3D12_STENCIL_OP_REPLACE, // kReplace
	D3D12_STENCIL_OP_INCR_SAT, // kIncrSat
	D3D12_STENCIL_OP_DECR_SAT, // kDecrSat
	D3D12_STENCIL_OP_INVERT, // kInvert
	D3D12_STENCIL_OP_INCR, // kIncrWrap
	D3D12_STENCIL_OP_DECR, // kDecrWrap
};


D3D12_STENCIL_OP GetD3D12StencilOp(StencilOp stencilOp)
{
	return kStencilOps[u32(stencilOp)];
}


D3D12_FILL_MODE GetD3D12FillMode(FillMode fillMode)
{
	return fillMode == FillMode::kSolid ? D3D12_FILL_MODE_SOLID : D3D12_FILL_MODE_WIREFRAME;
}


constexpr D3D12_CULL_MODE kCullModes[] =
{
	D3D12_CULL_MODE_NONE,
	D3D12_CULL_MODE_FRONT,
	D3D12_CULL_MODE_BACK,
	D3D12_CULL_MODE_BACK,
};


D3D12_CULL_MODE GetD3D12CullMode(CullMode cullMode)
{
	BV_ASSERT(cullMode != CullMode::kFrontAndBack, "D3D12 Doesn't support front-and-back culling");

	return kCullModes[u32(cullMode)];
}


D3D12_COMMAND_LIST_TYPE GetD3D12CommandListType(CommandType commandType)
{
	switch (commandType)
	{
	case CommandType::kGraphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
	case CommandType::kCompute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
	case CommandType::kTransfer: return D3D12_COMMAND_LIST_TYPE_COPY;
	case CommandType::kVideoDecode: return D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE;
	case CommandType::kVideoEncode: return D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE;
	default: return D3D12_COMMAND_LIST_TYPE_NONE;
	}
}


constexpr D3D_PRIMITIVE_TOPOLOGY kTopologies[] =
{
	D3D_PRIMITIVE_TOPOLOGY_UNDEFINED, // kUndefined
	D3D_PRIMITIVE_TOPOLOGY_POINTLIST, // kPointList
	D3D_PRIMITIVE_TOPOLOGY_LINELIST, // kLineList
	D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, // kLineStrip
	D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, // kTriangleList
	D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, // kTriangleStrip
	D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN, // kTriangleFan
	D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ, // kLineListAdj
	D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ, // kLineStripAdj
	D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ, // kTriangleListAdj
	D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ, // kTriangleStripAdj
	D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST, // kPatchList
};


D3D_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopology(Topology topology, u32 patchControlPoints)
{
	return topology != Topology::kPatchList ? kTopologies[u32(topology)] : D3D_PRIMITIVE_TOPOLOGY((kTopologies[u32(topology)] + patchControlPoints) - 1);
}


constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE kTopologyTypes[] =
{
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED, // kUndefined
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, // kPointList
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, // kLineList
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, // kLineStrip
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, // kTriangleList
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, // kTriangleStrip
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, // kTriangleFan
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, // kLineListAdj
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, // kLineStripAdj
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, // kTriangleListAdj
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, // kTriangleStripAdj
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH, // kPatchList
};


D3D12_PRIMITIVE_TOPOLOGY_TYPE GetD3D12PrimitiveTopologyType(Topology topology)
{
	return kTopologyTypes[u32(topology)];
}


constexpr D3D12_SRV_DIMENSION kSRVDimensions[][2] =
{
	{ D3D12_SRV_DIMENSION_UNKNOWN, D3D12_SRV_DIMENSION_UNKNOWN },
	{ D3D12_SRV_DIMENSION_TEXTURE1D, D3D12_SRV_DIMENSION_TEXTURE1D },
	{ D3D12_SRV_DIMENSION_TEXTURE1DARRAY, D3D12_SRV_DIMENSION_TEXTURE1DARRAY },
	{ D3D12_SRV_DIMENSION_TEXTURE2D, D3D12_SRV_DIMENSION_TEXTURE2DMS },
	{ D3D12_SRV_DIMENSION_TEXTURE2DARRAY, D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY },
	{ D3D12_SRV_DIMENSION_TEXTURECUBE, D3D12_SRV_DIMENSION_TEXTURECUBE },
	{ D3D12_SRV_DIMENSION_TEXTURECUBEARRAY, D3D12_SRV_DIMENSION_TEXTURECUBEARRAY },
	{ D3D12_SRV_DIMENSION_TEXTURE3D, D3D12_SRV_DIMENSION_TEXTURE3D },
};


D3D12_SRV_DIMENSION GetD3D12SRVDimension(TextureViewType textureViewType, bool multisample)
{
	return kSRVDimensions[u32(textureViewType)][u32(multisample)];
}


constexpr D3D12_UAV_DIMENSION kUAVDimensions[][2] =
{
	{ D3D12_UAV_DIMENSION_UNKNOWN, D3D12_UAV_DIMENSION_UNKNOWN },
	{ D3D12_UAV_DIMENSION_TEXTURE1D, D3D12_UAV_DIMENSION_TEXTURE1D },
	{ D3D12_UAV_DIMENSION_TEXTURE1DARRAY, D3D12_UAV_DIMENSION_TEXTURE1DARRAY },
	{ D3D12_UAV_DIMENSION_TEXTURE2D, D3D12_UAV_DIMENSION_TEXTURE2DMS },
	{ D3D12_UAV_DIMENSION_TEXTURE2DARRAY, D3D12_UAV_DIMENSION_TEXTURE2DMSARRAY },
	{ D3D12_UAV_DIMENSION_UNKNOWN, D3D12_UAV_DIMENSION_UNKNOWN },
	{ D3D12_UAV_DIMENSION_UNKNOWN, D3D12_UAV_DIMENSION_UNKNOWN },
	{ D3D12_UAV_DIMENSION_TEXTURE3D, D3D12_UAV_DIMENSION_TEXTURE3D },
};


D3D12_UAV_DIMENSION GetD3D12UAVDimension(TextureViewType textureViewType, bool multisample)
{
	return kUAVDimensions[u32(textureViewType)][u32(multisample)];
}


D3D12_CONSTANT_BUFFER_VIEW_DESC GetD3D12CBVDesc(const BufferViewDesc& viewDesc)
{
	BV_ASSERT(viewDesc.m_pBuffer != nullptr, "Invalid buffer handle");

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbv{};
	cbv.BufferLocation = viewDesc.m_pBuffer->GetDeviceAddress() + viewDesc.m_Offset;
	cbv.SizeInBytes = viewDesc.m_Stride * viewDesc.m_ElementCount;

	return cbv;
}


D3D12_SHADER_RESOURCE_VIEW_DESC GetD3D12SRVDesc(const BufferViewDesc& viewDesc)
{
	BV_ASSERT(viewDesc.m_pBuffer != nullptr, "Invalid buffer handle");

	D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
	srv.Format = DXGI_FORMAT(viewDesc.m_Format);
	srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv.Buffer = { 0, viewDesc.m_ElementCount, viewDesc.m_Stride, D3D12_BUFFER_SRV_FLAG_NONE };

	return srv;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC GetD3D12UAVDesc(const BufferViewDesc& viewDesc)
{
	BV_ASSERT(viewDesc.m_pBuffer != nullptr, "Invalid buffer handle");

	D3D12_UNORDERED_ACCESS_VIEW_DESC uav{};
	uav.Format = DXGI_FORMAT(viewDesc.m_Format);
	uav.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uav.Buffer = { 0, viewDesc.m_ElementCount, viewDesc.m_Stride, 0, D3D12_BUFFER_UAV_FLAG_NONE };

	return uav;
}


D3D12_SHADER_RESOURCE_VIEW_DESC GetD3D12SRVDesc(const TextureViewDesc& viewDesc)
{
	BV_ASSERT(viewDesc.m_pTexture != nullptr, "Invalid texture handle");

	auto pTex = TO_D3D12(viewDesc.m_pTexture);

	D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
	srv.Format = DXGI_FORMAT(viewDesc.m_Format);
	srv.ViewDimension = GetD3D12SRVDimension(viewDesc.m_ViewType, pTex->GetDesc().m_SampleCount > 1);
	srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	auto& subres = viewDesc.m_SubresourceDesc;
	switch (srv.ViewDimension)
	{
	case D3D12_SRV_DIMENSION_TEXTURE1D:
		srv.Texture1D = { subres.firstMip, subres.mipCount, 0.0f };
		break;
	case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
		srv.Texture1DArray = { subres.firstMip, subres.mipCount, subres.firstLayer, subres.layerCount, 0.0f };
		break;
	case D3D12_SRV_DIMENSION_TEXTURE2D:
		srv.Texture2D = { subres.firstMip, subres.mipCount, subres.planeSlice, 0.0f };
		break;
	case D3D12_SRV_DIMENSION_TEXTURE2DMS:
		srv.Texture2DMS = {};
		break;
	case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
		srv.Texture2DArray = { subres.firstMip, subres.mipCount, subres.firstLayer, subres.layerCount, subres.planeSlice, 0.0f };
		break;
	case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
		srv.Texture2DMSArray = { subres.firstLayer, subres.layerCount };
		break;
	case D3D12_SRV_DIMENSION_TEXTURECUBE:
		srv.TextureCube = { subres.firstMip, subres.mipCount, 0.0f };
		break;
	case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
		srv.TextureCubeArray = { subres.firstMip, subres.mipCount, subres.firstLayer, subres.layerCount, 0.0f };
		break;
	case D3D12_SRV_DIMENSION_TEXTURE3D:
		srv.Texture3D = { subres.firstMip, subres.mipCount, 0.0f };
		break;
	}

	return srv;
}


D3D12_UNORDERED_ACCESS_VIEW_DESC GetD3D12UAVDesc(const TextureViewDesc& viewDesc)
{
	BV_ASSERT(viewDesc.m_pTexture != nullptr, "Invalid texture handle");

	auto pTex = TO_D3D12(viewDesc.m_pTexture);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uav{};
	uav.Format = DXGI_FORMAT(viewDesc.m_Format);
	uav.ViewDimension = GetD3D12UAVDimension(viewDesc.m_ViewType, pTex->GetDesc().m_SampleCount > 1);

	auto& subres = viewDesc.m_SubresourceDesc;
	switch (uav.ViewDimension)
	{
	case D3D12_UAV_DIMENSION_TEXTURE1D:
		uav.Texture1D = { subres.firstMip };
		break;
	case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
		uav.Texture1DArray = { subres.firstMip, subres.firstLayer, subres.layerCount };
		break;
	case D3D12_UAV_DIMENSION_TEXTURE2D:
		uav.Texture2D = { subres.firstMip, subres.planeSlice };
		break;
	case D3D12_UAV_DIMENSION_TEXTURE2DMS:
		uav.Texture2DMS = {};
		break;
	case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
		uav.Texture2DArray = { subres.firstMip, subres.firstLayer, subres.layerCount, subres.planeSlice };
		break;
	case D3D12_UAV_DIMENSION_TEXTURE2DMSARRAY:
		uav.Texture2DMSArray = { subres.firstLayer, subres.layerCount };
		break;
	case D3D12_UAV_DIMENSION_TEXTURE3D:
		uav.Texture3D = { subres.firstMip, subres.firstLayer, subres.layerCount };
		break;
	}

	return uav;
}


D3D12_QUERY_HEAP_TYPE GetD3D12QueryHeapType(QueryType queryType)
{
	constexpr D3D12_QUERY_HEAP_TYPE kQueryTypes[] =
	{
		D3D12_QUERY_HEAP_TYPE_TIMESTAMP,
		D3D12_QUERY_HEAP_TYPE_OCCLUSION,
		D3D12_QUERY_HEAP_TYPE_OCCLUSION,
		D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS,
		D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS1,
	};

	return kQueryTypes[u32(queryType)];
}


D3D12_QUERY_TYPE GetD3D12QueryType(QueryType queryType)
{
	constexpr D3D12_QUERY_TYPE kQueryTypes[] =
	{
		D3D12_QUERY_TYPE_TIMESTAMP,
		D3D12_QUERY_TYPE_OCCLUSION,
		D3D12_QUERY_TYPE_BINARY_OCCLUSION,
		D3D12_QUERY_TYPE_PIPELINE_STATISTICS,
		D3D12_QUERY_TYPE_PIPELINE_STATISTICS1,
	};

	return kQueryTypes[u32(queryType)];
}
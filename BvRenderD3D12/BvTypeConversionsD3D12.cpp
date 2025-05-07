#include "BvTypeConversionsD3D12.h"


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
	}
}
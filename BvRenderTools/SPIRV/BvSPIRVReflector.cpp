#include "BvSPIRVReflector.h"
#include "External/SPIRV-Reflect/spirv_reflect.h"
#include "External/Vulkan-Headers/include/vulkan/vulkan.h"


class BvSPIRVShaderReflectionData : IBvShaderReflectionData
{
	BV_NOCOPYMOVE(BvSPIRVShaderReflectionData);

public:
	BvSPIRVShaderReflectionData() {}
	~BvSPIRVShaderReflectionData() {}

	const ShaderResourceLayoutDesc& GetResourceLayout() const override
	{
		return m_ShaderResourceLayoutDesc;
	}

	const BvVector<VertexInputDesc>& GetVertexAttributes() const override
	{
		return m_VertexAttributes;
	}

private:
	ShaderResourceLayoutDesc m_ShaderResourceLayoutDesc;
	BvVector<VertexInputDesc> m_VertexAttributes;
};


IBvShaderReflectionData* BvSPIRVReflector::Reflect(const u8* pBlob, const u32 blobSize, ShaderLanguage shaderLanguage) const
{
	SpvReflectShaderModule shaderModule{};
	auto result = spvReflectCreateShaderModule(blobSize, pBlob, &shaderModule);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	u32 count = 0;
	result = spvReflectEnumerateDescriptorSets(&shaderModule, &count, nullptr);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	BvVector<SpvReflectDescriptorSet*> sets(count);
	result = spvReflectEnumerateDescriptorSets(&shaderModule, &count, sets.Data());
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	BvVector<ShaderResourceLayoutDesc> setLayouts(sets.Size(), {});
	for (size_t setIndex = 0; setIndex < sets.Size(); ++setIndex)
	{
		const SpvReflectDescriptorSet& currSet = *(sets[setIndex]);
		for (uint32_t bindingIndex = 0; bindingIndex < currSet.binding_count; ++bindingIndex)
		{
			const SpvReflectDescriptorBinding& currBinding = *(currSet.bindings[bindingIndex]);
		}
	}

	spvReflectDestroyShaderModule(&shaderModule);

	return nullptr;
}
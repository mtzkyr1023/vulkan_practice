
#include "descriptor.h"
#include "../render_engine.h"


static const vk::DescriptorType g_descriptorType[] =
{
	vk::DescriptorType::eUniformBuffer,
};

void DescriptorSetLayout::setup(RenderEngine* engine)
{
	for (const auto& binding : bindings_)
	{
		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindings(binding.second);

		layouts_.push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
	}
}

void DescriptorSetLayout::cleanup(RenderEngine* engine)
{
	for (const auto& layout : layouts_)
	{
		engine->device().destroyDescriptorSetLayout(layout);
	}
}

void DescriptorSetLayout::addBinding(EDescriptorSetID setIndex, uint32_t bindingIndex, uint32_t descriptorCount, vk::ShaderStageFlags shaderStageFlags)
{
	vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
		.setBinding(bindingIndex)
		.setDescriptorType(g_descriptorType[(uint32_t)setIndex])
		.setDescriptorCount(descriptorCount)
		.setStageFlags(shaderStageFlags);

	bindings_[setIndex].push_back(binding);
}
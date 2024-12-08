#ifndef _DESCRIPTOR_H_
#define _DESCRIPTOR_H_

#include <vulkan/vulkan.hpp>

#include <unordered_map>

enum class EDescriptorSetID
{
	eUniformBuffer = 0,
};

class DescriptorSetLayout {
public:
	DescriptorSetLayout() {}
	~DescriptorSetLayout() {}

	void setup(class RenderEngine* engine);
	void cleanup(class RenderEngine* engine);

	void addBinding(EDescriptorSetID setIndex, uint32_t bindingIndex, uint32_t descriptorCount, vk::ShaderStageFlags shaderStageFlags);

	const std::vector<vk::DescriptorSetLayout> layouts() { return layouts_; }

private:
	std::vector<vk::DescriptorSetLayout> layouts_;
	std::unordered_map<EDescriptorSetID, std::vector<vk::DescriptorSetLayoutBinding>> bindings_;
};

#endif
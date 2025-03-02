#ifndef _RENDER_PIPELINE_H_
#define _RENDER_PIPELINE_H_

#include <vulkan/vulkan.hpp>
#include <shaderc/shaderc.hpp>

#include "../gameobject/gameobject.h"
#include "resource.h"

class RenderPipeline {
public:
	virtual ~RenderPipeline() {}

	virtual void initialize(class RenderEngine* engine, class RenderPass* pass) = 0;
	virtual void cleanup(class RenderEngine* engine);
	
	virtual void render(class RenderEngine* engine, class RenderPass* pass, uint32_t currentImageIndex) = 0;
	virtual void update(class RenderEngine* engine, uint32_t currentImageIndex) = 0;

	vk::CommandBuffer commandBuffer(uint32_t currentFrameIndex) { return commandBuffers_[currentFrameIndex]; }
	vk::Semaphore renderCompletedSemaphore(uint32_t currentFrameIndex) { return renderCompletedSemaphores_[currentFrameIndex]; }

protected:
	vk::ShaderModule createShaderModule(
		class RenderEngine* engine,
		const std::string& filename,
		const std::string& entryPoint,
		shaderc::CompileOptions options,
		shaderc_shader_kind shaderKind);

protected:
	std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayout>> descriptorLayouts_;
	std::unordered_map<uint32_t, std::vector<vk::DescriptorSet>> sets_;
	std::unordered_map<uint32_t, vk::PipelineLayout> pipelineLayout_;
	std::unordered_map<uint32_t, vk::Pipeline> pipeline_;
	vk::Viewport viewport_;
	std::vector<vk::CommandBuffer> commandBuffers_;
	std::vector<vk::Semaphore> renderCompletedSemaphores_;

	static GameObjectManager objectManager_;
};

#endif
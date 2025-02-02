#include "simple_pipeline.h"
#include "../../render_engine.h"
#include "../../defines.h"
#include "../../gameobject/camera.h"
#include "assimp/Importer.hpp"
#include "../../util/timer.h"

#undef MemoryBarrier

SimplePipeline::SimplePipeline() :
	mappedViewProjMemory_(nullptr)
{
	Assimp::Importer* imp = new Assimp::Importer();

	delete imp;
}

SimplePipeline::~SimplePipeline() {

}

void SimplePipeline::initialize(RenderEngine* engine) {
	{
		vk::DescriptorSetLayoutBinding binding = vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);

		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setBindings({ binding });

		layouts_.push_back(engine->device().createDescriptorSetLayout(layoutCreateInfo));
	}

	{
		viewport_ = vk::Viewport()
			.setWidth((float)kScreenWidth)
			.setHeight((float)kScreenHeight)
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f)
			.setX(0.0f)
			.setY(0.0f);

		vk::Rect2D scissor = vk::Rect2D()
			.setExtent(vk::Extent2D(kScreenWidth, kScreenHeight))
			.setOffset(vk::Offset2D(0, 0));

		vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo()
			.setScissorCount(1)
			.setPScissors(&scissor)
			.setViewportCount(1)
			.setPViewports(&viewport_);
		
		vk::PipelineVertexInputStateCreateInfo vertexInputState = vk::PipelineVertexInputStateCreateInfo()
			.setVertexAttributeDescriptionCount(0)
			.setPVertexAttributeDescriptions(nullptr)
			.setVertexBindingDescriptionCount(0)
			.setPVertexBindingDescriptions(nullptr);

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);

		vk::PipelineRasterizationStateCreateInfo rasterizationState = vk::PipelineRasterizationStateCreateInfo()
			.setCullMode(vk::CullModeFlagBits::eNone)
			.setDepthBiasClamp(0.0f)
			.setDepthBiasConstantFactor(0.0f)
			.setDepthBiasEnable(vk::False)
				.setDepthBiasSlopeFactor(0.0f)
				.setDepthClampEnable(vk::False)
				.setFrontFace(vk::FrontFace::eCounterClockwise)
				.setLineWidth(1.0f)
				.setPolygonMode(vk::PolygonMode::eFill)
				.setRasterizerDiscardEnable(vk::False);

		vk::PipelineMultisampleStateCreateInfo multiSampleState = vk::PipelineMultisampleStateCreateInfo()
			.setAlphaToCoverageEnable(vk::False)
			.setAlphaToOneEnable(vk::False)
			.setMinSampleShading(0.0f)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setSampleShadingEnable(vk::False);

		vk::PipelineColorBlendAttachmentState colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState()
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA)
			.setBlendEnable(vk::False);

		vk::PipelineColorBlendStateCreateInfo colorBlendState = vk::PipelineColorBlendStateCreateInfo()
			.setAttachmentCount(1)
			.setLogicOpEnable(vk::False)
			.setPAttachments(&colorBlendAttachmentState);

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(layouts_);

		pipelineLayout_ = engine->device().createPipelineLayout(pipelineLayoutCreateInfo);

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(2);
		shaderStages[0].setStage(vk::ShaderStageFlagBits::eVertex);
		shaderStages[0].setPName("main");
		shaderStages[0].setModule(
			createShaderModule(engine, "shaders/simple.vert", "main", shaderc::CompileOptions(), shaderc_vertex_shader));
		shaderStages[1].setStage(vk::ShaderStageFlagBits::eFragment);
		shaderStages[1].setPName("main");
		shaderStages[1].setModule(
			createShaderModule(engine, "shaders/simple.frag", "main", shaderc::CompileOptions(), shaderc_fragment_shader));

		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			.setPViewportState(&viewportState)
			.setPVertexInputState(&vertexInputState)
			.setPInputAssemblyState(&inputAssemblyState)
			.setPRasterizationState(&rasterizationState)
			.setPMultisampleState(&multiSampleState)
			.setPColorBlendState(&colorBlendState)
			.setLayout(pipelineLayout_)
			.setStageCount(2)
			.setStages(shaderStages)
			.setRenderPass(engine->renderPass())
			.setSubpass(0);

		vk::ResultValue<vk::Pipeline> result = engine->device().createGraphicsPipeline(
			engine->pipelineCache(),
			graphicsPipelineCreateInfo);

		pipeline_ = result.value;

		engine->device().destroyShaderModule(shaderStages[0].module);
		engine->device().destroyShaderModule(shaderStages[1].module);
	}

	commandBuffers_ = engine->allocateCommandBuffer(engine->swapchainImageCount());

	for (uint32_t i = 0; i < (uint32_t)engine->images().size(); i++) {
		renderCompletedSemaphores_.push_back(engine->device().createSemaphore(vk::SemaphoreCreateInfo()));
	}

	ubMemory.allocateForBuffer(
		engine->physicalDevice(),
		engine->device(),
		vk::BufferCreateInfo()
		.setSize(sizeof(glm::mat4) * 4 * engine->swapchainImageCount())
		.setUsage(vk::BufferUsageFlagBits::eUniformBuffer),
		vk::MemoryPropertyFlagBits::eHostVisible);
	for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setSize(sizeof(glm::mat4) * 4)
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);

		viewProjBuffer_.push_back(engine->device().createBuffer(bufferCreateInfo));
	}

	for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
	{
		ubMemory.bind(engine->device(), viewProjBuffer_[i], sizeof(glm::mat4) * 4 * i);	
	}

	mappedViewProjMemory_ = ubMemory.map(engine->device(), 0, sizeof(glm::mat4) * 4 * engine->swapchainImageCount());


	{
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(engine->descriptorPool())
			.setSetLayouts(layouts_);

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			sets_.push_back(engine->device().allocateDescriptorSets(allocInfo)[0]);
		}

		std::vector<vk::WriteDescriptorSet> writes;

		for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
		{
			vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo()
				.setBuffer(viewProjBuffer_[i])
				.setOffset(0)
				.setRange(sizeof(glm::mat4) * 4);

			writes.push_back(vk::WriteDescriptorSet()
				.setBufferInfo(bufferInfo)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDstArrayElement(0)
				.setDstBinding(0)
				.setDstSet(sets_[i]));
		}

		engine->device().updateDescriptorSets(writes, {});
	}
}

void SimplePipeline::cleanup(RenderEngine* engine) {
	RenderPipeline::cleanup(engine);

	for (uint32_t i = 0; i < engine->swapchainImageCount(); i++)
	{
		engine->device().destroyBuffer(viewProjBuffer_[i]);
	}

	ubMemory.free(engine->device());

	engine->device().freeDescriptorSets(engine->descriptorPool(), sets_);
	for (const auto& layout : layouts_)
	{
		engine->device().destroyDescriptorSetLayout(layout);
	}
}

void SimplePipeline::render(RenderEngine* engine, uint32_t currentImageIndex) {
	vk::CommandBuffer cb = engine->commandBuffer(currentImageIndex);

	vk::ClearValue clearValues[1] = {
		vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f),
	};

	{
		vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		std::vector<vk::ImageMemoryBarrier> barriers(1);
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[0].setImage(engine->images()[currentImageIndex]);
		barriers[0].setOldLayout(vk::ImageLayout::ePresentSrcKHR);
		barriers[0].setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(subresourceRange);

		//cb.pipelineBarrier(
		//	vk::PipelineStageFlagBits::eTransfer,
		//	vk::PipelineStageFlagBits::eColorAttachmentOutput,
		//	vk::DependencyFlagBits::eDeviceGroup,
		//	nullptr,
		//	nullptr,
		//	barriers);
	}

	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);

	cb.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout_,
		0,
		{ sets_[currentImageIndex] },
		{});

	cb.draw(3, 1, 0, 0);


	{
		vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		std::vector<vk::ImageMemoryBarrier> barriers(1);
		barriers[0].setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barriers[0].setDstAccessMask(vk::AccessFlagBits::eTransferRead);
		barriers[0].setImage(engine->images()[currentImageIndex]);
		barriers[0].setOldLayout(vk::ImageLayout::eColorAttachmentOptimal);
		barriers[0].setNewLayout(vk::ImageLayout::ePresentSrcKHR);
		barriers[0].setSrcQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setDstQueueFamilyIndex(engine->graphicsQueueFamilyIndex());
		barriers[0].setSubresourceRange(subresourceRange);

		//cb.pipelineBarrier(
		//	vk::PipelineStageFlagBits::eColorAttachmentOutput,
		//	vk::PipelineStageFlagBits::eTransfer,
		//	vk::DependencyFlagBits::eDeviceGroup,
		//	nullptr,
		//	nullptr,
		//	barriers);
	}
}

void SimplePipeline::update(RenderEngine* engine, uint32_t currentImageIndex)
{
	struct ViewProj
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 world;
	};
	
	ViewProj vp;

	static float rot = 0.0f;

	rot += glm::radians<float>(Timer::instance().deltaTime() * 360.0f);

	vp.world = glm::mat4(glm::rotate(glm::identity<glm::quat>(), rot, glm::vec3(0.0f, 1.0f, 0.0f)));
	vp.view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, -5.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	vp.proj = glm::perspectiveFov(
		glm::pi<float>() * 0.5f,
		(float)kScreenWidth,
		(float)kScreenHeight,
		0.1f,
		1000.0f);

	memcpy_s(
		&mappedViewProjMemory_[sizeof(glm::mat4) * 4 * currentImageIndex],
		sizeof(ViewProj),
		&vp,
		sizeof(ViewProj));
}
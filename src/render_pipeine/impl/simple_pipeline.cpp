#include "simple_pipeline.h"
#include "../../render_engine.h"
#include "../../defines.h"
#include "../../gameobject/camera.h"


#undef MemoryBarrier

SimplePipeline::SimplePipeline() {

}

SimplePipeline::~SimplePipeline() {

}

void SimplePipeline::initialize(RenderEngine* engine) {
	{
		vk::AttachmentReference attachmentRefs = vk::AttachmentReference()
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::AttachmentDescription attachmentDesc = vk::AttachmentDescription()
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFormat(engine->backbufferFormat())
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

		vk::SubpassDescription subpassDesc = vk::SubpassDescription()
			.setColorAttachmentCount(1)
			.setPColorAttachments(&attachmentRefs)
			.setInputAttachmentCount(0)
			.setPInputAttachments(nullptr)
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

		vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo()
			.setAttachmentCount(1)
			.setPAttachments(&attachmentDesc)
			.setDependencyCount(0)
			.setSubpassCount(1)
			.setPSubpasses(&subpassDesc);

		renderPass_ = engine->device().createRenderPass(renderPassCreateInfo);
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
			.setCullMode(vk::CullModeFlagBits::eBack)
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
			.setSetLayoutCount(0)
			.setPSetLayouts(nullptr);

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
			.setRenderPass(renderPass_)
			.setSubpass(0);

		vk::ResultValue<vk::Pipeline> result = engine->device().createGraphicsPipeline(
			engine->pipelineCache(),
			graphicsPipelineCreateInfo);

		pipeline_ = result.value;

		engine->device().destroyShaderModule(shaderStages[0].module);
		engine->device().destroyShaderModule(shaderStages[1].module);
	}

	{
		for (size_t i = 0; i < engine->imageViews().size(); i++) {
			vk::ImageView imageViews[1] = {
				engine->imageViews()[i],
			};
			vk::FramebufferCreateInfo framebufferCreateInfo = vk::FramebufferCreateInfo()
				.setAttachmentCount(1)
				.setPAttachments(imageViews)
				.setWidth((uint32_t)kScreenWidth)
				.setHeight((uint32_t)kScreenHeight)
				.setRenderPass(renderPass_)
				.setLayers(1);

			framebuffers_.push_back(engine->device().createFramebuffer(framebufferCreateInfo));
		}
	}

	commandBuffers_ = engine->allocateCommandBuffer((uint32_t)engine->images().size());
	
	viewProjBuffer_ = resourceManager_.getBuffer(0);
	viewProjBuffer_->createUniformBuffer(engine, sizeof(glm::mat4) * 8);

	for (uint32_t i = 0; i < (uint32_t)engine->images().size(); i++) {
		renderCompletedSemaphores_.push_back(engine->device().createSemaphore(vk::SemaphoreCreateInfo()));
	}
}

void SimplePipeline::cleanup(RenderEngine* engine) {
	RenderPipeline::cleanup(engine);

	for (vk::Framebuffer& framebuffer : framebuffers_) {
		engine->device().destroyFramebuffer(framebuffer);
	}
}

void SimplePipeline::render(RenderEngine* engine, uint32_t currentImageIndex) {
	vk::CommandBuffer cb = commandBuffers_[0];

	cb.begin(vk::CommandBufferBeginInfo());

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

	vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setRenderPass(renderPass_)
		.setFramebuffer(framebuffers_[currentImageIndex])
		.setRenderArea(vk::Rect2D({ 0, 0 }, { kScreenWidth, kScreenHeight }))
		.setClearValueCount(1)
		.setPClearValues(clearValues);

	cb.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);

	cb.draw(3, 1, 0, 0);

	cb.endRenderPass();


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

	cb.end();
}